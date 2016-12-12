#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "mapreduce.h"
#include "linkedlist.h"


/**
 * The Worker is a record for a single worker process with bidirectional
 * communication between the parent and the child. The parent sends data
 * down the `input` file descriptor, and the child sends data back over
 * the `output` descriptor. `pid` stores the child process ID.
 */
typedef struct worker {
    int input;
    int output;
    int pid;
} Worker;

/**
 * The App is the primary structure used after argument parsing. It holds
 * the map workers, reduce workers, and their file descriptors.
 */
typedef struct app {
    // Target directory to search for files in.
    char *target;

    // MapWorkers and ReduceWorkers are the count of map and reduce worker.
    // The length of workers and reduces corresponds to the count
    // stored in these.
    int mapWorkers;
    int reduceWorkers;

    Worker *mappers;
    Worker *reducers;

    // Intermediary storage of kv pairs between the map and reduce step.
    LLKeyValues *kv;
} App;

/**
 * Sets the directory to search for text files.
 * @param app the application to operate on
 * @param dir the directory to add
 */
void set_target_dir(App *app, char *dir)
{
    int len = strlen(dir);

    app->target = checked_malloc(sizeof(char) * (len + 1));
    strcpy(app->target, dir);
    app->target[len] = '\0';
}

/**
 * Determines whether the application is in a state such that it can start
 * running the map-reduce algorithm.
 * @param  app the target application
 * @return 1 if it's OK, zero otherwise.
 */
int is_valid(App *app)
{
    if (app->target == NULL) {
        return 0;
    }

    if (app->mapWorkers <= 0 || app->reduceWorkers <= 0) {
        return 0;
    }

    return 1;
}

/**
 * @returns an instance of the App struct. This is allocated on the heap and
 * should be deallocated with free_app(app) once it's finished being used.
 */
App *make_app()
{
    App *app = checked_malloc(sizeof(App));
    memset(app, 0, sizeof(App));
    app->mapWorkers = 2;
    app->reduceWorkers = 2;

    return app;
}

/**
 * Free clears memory allocated to the application.
 * @param app the application to deallocate.
 */
void free_app(App *app)
{
    free_key_values_list(app->kv);
    free(app->target);
    free(app->mappers);
    free(app->reducers);
    free(app);
}

/**
 * Runs the `ls` commands and pieces the files out to the map workers
 * in the application. When this is called it's expected that the
 * map workers will be spawned and awaiting input.
 * @param app the application container
 */
void dispatch_lists(App *app)
{
    int fd[2];
    pipe(fd);

    int pid = checked_fork();
    if (pid == 0) {
        // Child process:
        // Close the output file descriptor, and replace our stdout with
        // the output side of the pipe. Then replace this process with ls.
        close(fd[0]);
        dup2(fd[1], 1);
        execl("/bin/ls", "ls", app->target, "-1");
        perror("execv");
        return;
    }

    // Set up the process to scan MAX_FILENAME-byte strings from the
    // child process on the other side of the pipe.
    char scanFmt[10];
    sprintf(scanFmt, "%%%ds", MAX_FILENAME);
    char file[MAX_FILENAME] = {'\0'};
    close(fd[1]);
    dup2(fd[0], 0);

    // Scan over the results and round-robin them out to map workers.
    for (int i = 0; scanf(scanFmt, file) != EOF; i++) {
        Worker worker = app->mappers[i % app->mapWorkers];
        write(worker.input, file, MAX_FILENAME * sizeof(char));
    }

    assert_fork_end(pid, "ls");
}

/**
 * Doles pairs out to previously-spawned reducer functions.
 * @param app the application context, containing reducesrs.
 */
void dispatch_reducers(App *app)
{
    LLKeyValues *list = app->kv;
    LLValues *value;
    Pair pair = {{'\0'}, {'\0'}};

    for (int i = 0; list != NULL; i++) {
        strncpy(pair.key, list->key, MAX_KEY);
        for (value = list->head_value; value != NULL; value = value->next) {
            Worker worker = app->reducers[i % app->reduceWorkers];
            strncpy(pair.value, value->value, MAX_KEY);
            write(worker.input, &pair, sizeof(Pair));
        }

        list = list->next;
    }

    for (int i = 0; i < app->reduceWorkers; i++) {
        Worker w = app->reducers[i];
        close(w.input);
        close(w.output);
        assert_fork_end(w.pid, "reducer");
    }
}

/**
 * Runs the map function, using the provided in/out file descriptors.
 * @param in  file descriptor that files will be sent down. Each file will
 *            be exactly MAX_FILENAME + 1 bytes long (terminated by a null
 *            byte).
 * @param out file descriptor you should push mapped data down.
 */
void run_map(App *app, int in, int out)
{
    // Assign the input/outputs of the pipe to our stdin and out
    dup2(in, 0);
    dup2(out, 1);

    // Target buffer for reading file data.
    char chunk[READSIZE] = {'\0'};

    // Character arrays big enough to hold file paths. We read up to MAX_FILENAME
    // characters into fname from stdin, then we concatenate that with the
    // file path into `path`. We have plus one on the path since we
    // may need to include space for a separator from path_join.
    char fname[MAX_FILENAME];
    char *path = checked_malloc((strlen(app->target) + MAX_FILENAME + 1) * sizeof(char));

    while (read(0, fname, MAX_FILENAME) > 0) {
        path_join(path, app->target, fname);
        FILE *fp = checked_fopen(path, "r");
        while (fread(chunk, READSIZE, 1, fp) == 1) {
            map(chunk, 1);
        }
        fclose(fp);
    }

    close(out);
    free(path);
}

/**
 * Runs the reduce function, using the provided in/out file descriptors. It'll
 * read pair data from the `in`, then write out the reduced pairs to
 * a file [pid].out in binary.
 * @param app the application context
 * @param in  the `in` pipe, where pairs will be written to
 * @param out output pipe (closed immediately)
 */
void run_reduce(App *app, int in, int out)
{
    close(out);

    LLKeyValues *list = NULL;
    Pair pair;
    while (read(in, &pair, sizeof(Pair)) > 0) {
        insert_into_keys(&list, pair);
    }

    char fname[MAX_FILENAME];
    sprintf(fname, "%d.out", getpid());
    FILE *fd = checked_fopen(fname, "wb");

    for (LLKeyValues *l = list; l != NULL; l = l->next) {
        pair = reduce(l->key, l->head_value);
        fwrite(&pair, sizeof(Pair), 1, fd);
    }

    free_key_values_list(list);
    fclose(fd);
}

/**
 * Spawns workers for the application.
 * @param count   the number of workers to spawn
 * @param workers pointer to an address where the list of Workers will be stored.
 * @param fn      the function to run on the worker. Takes an (input fd, output
 *                fd) as arguments. It is responsible for closing the input and
 *                output when it's done with them. The application will be
 *                freed and the process exited after the function returns.
 */
void spawn_workers(App *app, int count, Worker **workers, void(*fn)(App*, int, int))
{
    Worker *w = *workers = checked_malloc(count * sizeof(Worker));

    int pid;
    int parent_to_child[2];
    int child_to_parent[2];

    // Now for however many workers we requested, start forking processes.
    for (int i = 0; i < count; i++){
        pipe(parent_to_child);
        pipe(child_to_parent);

        pid = checked_fork();
        if (pid == 0) {
            // If we became a child, close the FDs we don't need AND the
            // fds of the other children we don't care about.
            close(parent_to_child[1]);
            close(child_to_parent[0]);
            for (int k = 0; k < i; k++) {
                close(w[k].input);
                close(w[k].output);
            }

            // Then call the worker function, passing the input and output fds.
            (*fn)(app, parent_to_child[0], child_to_parent[1]);
            free_app(app);
            exit(0);
            return;
        }

        // Otherwise, close the fds we don't need and add this worker to
        // the appropriate list.
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        w[i].input = parent_to_child[1];
        w[i].output = child_to_parent[0];
        w[i].pid = pid;
    }
}

/**
 * Aggregates and reduce the outputs from all map functions.
 * @param app the application context
 */
void collect_mappers(App *app)
{
    Worker w;
    Pair pair;
    for (int i = 0; i < app->mapWorkers; i++) {
        w = app->mappers[i];
        close(w.input);

        while (read(w.output, &pair, sizeof(Pair)) > 0) {
            insert_into_keys(&app->kv, pair);
        }

        assert_fork_end(w.pid, "mapper");
    }
}

/**
 * Runs the map reduce algorithm based on the current application settings.
 * @param app the application container
 */
void run_mr(App *app)
{
    if (!is_valid(app)) {
        printf("Attempted to run_mr on an invalid application!\n");
        exit(1);
    }

    spawn_workers(app, app->mapWorkers, &app->mappers, run_map);
    dispatch_lists(app);
    collect_mappers(app);
    spawn_workers(app, app->reduceWorkers, &app->reducers, run_reduce);
    dispatch_reducers(app);
}

/**
 * Prints usage information for the program then exits with a non-zero
 * status code.
 */
void print_usage_and_exit()
{
    printf(
        "MapReduce v0.1.0. Usage of mapreduce:\n"
        "\n"
        "  -m int\n"
        "    Specifies how many map workers to spawn. (default 2)\n"
        "  -r int\n"
        "    Specifies how many reduce workers to spawn. (default 2)\n"
        "  -d string\n"
        "    Specifies the absolute or relative path containing files to work on.\n"
    );
    exit(1);
}

int main(int argc, char *argv[])
{
    App *app = make_app();

    int opt;
    while ((opt = getopt(argc, argv, "m:r:d:")) != -1) {
        switch (opt) {
        case 'm': app->mapWorkers = atoi(optarg);    break;
        case 'r': app->reduceWorkers = atoi(optarg); break;
        case 'd': set_target_dir(app, optarg);      break;
        default:  print_usage_and_exit();
        }
    }

    if (!is_valid(app)) {
        print_usage_and_exit();
    } else {
        run_mr(app);
    }

    free_app(app);
    return 0;
}

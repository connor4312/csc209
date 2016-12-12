package main

import (
	"fmt"
	"io"
	"net"
	"time"
)

var script = []struct {
	connection int
	input      string
	output     string
}{
	{0, "", "What is your user name?\n"},
	{0, "user1\n", "Welcome.\nGo ahead and enter user commands>\n"},
	{1, "", "What is your user name?\n"},
	{1, "user2\n", "Welcome.\nGo ahead and enter user commands>\n"},

	// testing basic usage/args:
	{1, "asdf\n", "Incorrect syntax\n"},
	{1, "profile\n", "Incorrect syntax\n"},
	{1, "profile foo bar\n", "Incorrect syntax\n"},
	{1, "profile foo\n", "User not found\n"},
	{1, "profile user1\n", "Name: user1\n\n------------------------------------------\nFriends:\n------------------------------------------\nPosts:\n------------------------------------------\n"},
	{1, "list_users asdf\n", "Incorrect syntax\n"},
	{1, "list_users\n", "user1\nuser2\n"},
	{1, "make_friends\n", "Incorrect syntax\n"},
	{1, "make_friends foo bar\n", "Incorrect syntax\n"},
	{1, "make_friends foo\n", "The user you entered does not exist\n"},
	{1, "make_friends user2\n", "You can't friend yourself\n"},
	{1, "make_friends user1\n", "You are now friends with user1.\n"},
	{1, "make_friends user1\n", "You are already friends.\n"},
	{1, "profile user1\n", "Name: user1\n\n------------------------------------------\nFriends:\nuser2\n------------------------------------------\nPosts:\n------------------------------------------\n"},
	{1, "post\n", "Incorrect syntax\n"},
	{1, "post foo\n", "Incorrect syntax\n"},
	{1, "post foo bar\n", "The user you want to post to does not exist\n"},
	{1, "post user1 hello world\n", ""},

	// test reads partial commands and selects
	{0, "list_", ""},
	{1, "profile\n", "Incorrect syntax\n"},
	{0, "users\n", "user1\nuser2\n"},
}

const server = "127.0.0.1:58208"
const numConnections = 2

func panicIfErr(err error) {
	if err != nil {
		panic(err)
	}
}

/**
 * Quick and dirty integration test for the friendme server.
 */
func main() {
	connections := []net.Conn{}
	for i, s := range script {
		for s.connection >= len(connections) {
			cnx, _ := net.Dial("tcp", server)
			connections = append(connections, cnx)
		}

		cnx := connections[s.connection]
		cnx.SetDeadline(time.Now().Add(time.Minute))

		_, err := cnx.Write([]byte(s.input))
		panicIfErr(err)

		buf := make([]byte, len(s.output))
		_, err = io.ReadFull(cnx, buf)
		panicIfErr(err)

		if string(buf) != s.output {
			panic(fmt.Sprintf(
				"Running on step %d:\n\t%sExpected:\n\t%sGot:\n\t%s\n========\n",
				i,
				s.input,
				s.output,
				buf,
			))
		}
	}

	// now test it quits
	for _, cnx := range connections {
		cnx.SetDeadline(time.Now().Add(time.Minute))

		_, err := cnx.Write([]byte("quit\n"))
		panicIfErr(err)

		one := make([]byte, 1)
		if _, err := cnx.Read(one); err != io.EOF {
			panic(fmt.Sprintf("expected connection to close, got %s error instead", err))
		}
	}
}

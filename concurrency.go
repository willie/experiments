package main // main is always in package main

import ( // code you import from standard library or others
	"fmt"
	"time"
)

// fake slow computation
func slowComputation() {
	time.Sleep(1 * time.Second)
}

func main() {
	// use channels to send values between goroutines
	messages := make(chan string)

	// inline goroutine that will execute async to main
	go func() {
		slowComputation()
		messages <- "finished slowComputation from goroutine"
	}()

	// this call to slowComputation will block until finished
	slowComputation()
	fmt.Println("finished slowComputation in main")

	// block until a message is recieved
	msg := <-messages
	fmt.Println(msg)
}

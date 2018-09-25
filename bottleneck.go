package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

// fake slow computation
func slowComputation() {
	time.Sleep(time.Duration(rand.Intn(3)) * time.Second)
}

func main() {
	// channel size limits simultaneous execution
	concurrentUploads := 10
	limiter := make(chan struct{}, concurrentUploads)

	// we don't want to quit execution until all workers are done
	var workers sync.WaitGroup

	// assume big loop that needs to be processed with a slow function that could really slow things down
	for i := 0; i < 100; i++ {
		workers.Add(1)

		go func(w int) {
			defer workers.Done()

			limiter <- struct{}{}

			slowComputation()

			fmt.Println("worker #", w, "done")
			<-limiter
		}(i)

	}

	workers.Wait()
	fmt.Println("finished")
}

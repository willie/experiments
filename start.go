package main

import "fmt"

func main() {
	n := 3

	for x := 1; x <= 10; x = x + 1 {
		if (x % n) == 0 {
			fmt.Println(x % n)
		} else {
			fmt.Println(x / n)
		}
	}
}

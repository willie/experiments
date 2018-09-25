package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

func visit(path string, info os.FileInfo, err error) error {
	if err != nil {
		return err
	}

	if !info.Mode().IsRegular() {
		return nil
	}

	if info.IsDir() {
		return nil
	}

	matched, err := filepath.Match("*.c", info.Name())
	if err != nil {
		log.Fatalln(err)
	}

	if matched {
		read, err := ioutil.ReadFile(path)
		if err != nil {
			log.Fatalln(err)
		}

		source := string(read)

		if strings.Index(source, find) == -1 {
			return nil
		}

		fmt.Println(path)

		modified := strings.Replace(source, find, replace, -1)

		err = ioutil.WriteFile(path, []byte(modified), 0)
		if err != nil {
			log.Fatalln(err)
		}

	}

	return nil
}

var find = "#define FT_CONFIG_OPTION_SUBPIXEL_RENDERING"
var replace = "//#define FT_CONFIG_OPTION_SUBPIXEL_RENDERING"

func main() {
	err := filepath.Walk("pdfium", visit)
	if err != nil {
		log.Fatalln(err)
	}
}

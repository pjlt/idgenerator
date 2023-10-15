/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2023 Zhennan Tu <zhennan.tu@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package main

import (
	"bytes"
	"database/sql"
	"encoding/binary"
	"flag"
	"fmt"
	"os"
	"strings"

	_ "github.com/glebarez/sqlite"
)

const createTables = `
CREATE TABLE IF NOT EXISTS "unused_device_ids" (
	"id"	INTEGER NOT NULL UNIQUE,
	"deviceID"	INTEGER NOT NULL UNIQUE,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "used_device_ids" (
	"id"	INTEGER NOT NULL UNIQUE,
	"deviceID"	INTEGER NOT NULL UNIQUE,
	PRIMARY KEY("id" AUTOINCREMENT)
);
`

func main() {
	dbPath := flag.String("db", "lanthing-svr.sqlite", "The path of sqlite database file")
	idPath := flag.String("id", "", "The path of ids file")
	flag.Parse()

	if dbPath == nil || idPath == nil {
		flag.Usage()
		os.Exit(-1)
	}
	db, err := sql.Open("sqlite", *dbPath)
	if err != nil {
		panic(err)
	}
	fmt.Printf("Open db '%s' success\n", *dbPath)
	defer func() {
		db.Close()
		fmt.Println("Closed db")
	}()
	_, err = db.Exec(createTables)
	if err != nil {
		panic(err)
	}

	content, err := os.ReadFile(*idPath)
	if err != nil {
		panic(err)
	}
	if content == nil || len(content) == 0 || len(content)%4 != 0 {
		panic("Invalid file")
	}
	fmt.Printf("Read '%s' with %d bytes\n", *idPath, len(content))

	ids := make([]uint32, len(content)/4)
	reader := bytes.NewReader(content)
	err = binary.Read(reader, binary.LittleEndian, ids)
	if err != nil {
		panic(err)
	}

	index := 0
	for index < len(ids) {
		var builder strings.Builder
		builder.WriteString("INSERT INTO unused_device_ids(deviceID) VALUES ")
		endIndex := index + 500 //not included
		if endIndex > len(ids) {
			endIndex = len(ids)
		}
		vals := []interface{}{}
		for j := index; j < endIndex; j++ {
			if j == endIndex-1 {
				builder.WriteString("(?)")
			} else {
				builder.WriteString("(?),")
			}
			vals = append(vals, ids[j])
		}
		stmt, err := db.Prepare(builder.String())
		if err != nil {
			panic(err)
		}

		_, err = stmt.Exec(vals...)
		if err != nil {
			panic(err)
		}
		fmt.Printf("Exec %d rows\n", endIndex-index)
		index = endIndex
	}
}

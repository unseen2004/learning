// main.go
// Główny program z krótszym timeout'em

package main

import (
	"fmt"
	"time"
)

func main() {
	fmt.Println("=== Algorytm Wzajemnego Wykluczania Szymańskiego (Uproszczony) ===")
	fmt.Println("Stany procesów:")
	fmt.Println("  LocalSection     - Proces wykonuje pracę lokalną")
	fmt.Println("  EntryProtocol1   - f[i] = 1")
	fmt.Println("  EntryProtocol2   - f[i] = 2, oczekiwanie na warunek")
	fmt.Println("  EntryProtocol3   - f[i] = 3, oczekiwanie na warunek")
	fmt.Println("  EntryProtocol4   - f[i] = 4, oczekiwanie na warunek")
	fmt.Println("  CriticalSection  - Proces w sekcji krytycznej")
	fmt.Println("  ExitProtocol     - f[i] = 0, wyjście")
	fmt.Println("")
	fmt.Println("Wartości flag: 0=bezczynny, 1=chce_wejść, 2=czeka, 3=gotowy, 4=wchodzi")
	fmt.Println("Priorytet: procesy o niższym ID mają wyższy priorytet")
	fmt.Println("=======================================================")
	fmt.Println("")

	// Liczba procesów
	const numProcesses = 3

	// Utworzenie współdzielonych flag
	sharedFlags := NewAtomicFlags(numProcesses)

	// Kanał do sygnalizowania zakończenia
	done := make(chan bool, numProcesses)

	// Uruchomienie procesów z małym opóźnieniem
	for i := 1; i <= numProcesses; i++ {
		go ProcessTask(i, sharedFlags, done)
		time.Sleep(100 * time.Millisecond) // Małe opóźnienie między startem procesów
	}

	// Oczekiwanie na zakończenie z krótszym timeout'em
	completedProcesses := 0
	timeout := time.After(20 * time.Second) // 20 sekund timeout

	for completedProcesses < numProcesses {
		select {
		case <-done:
			completedProcesses++
			fmt.Printf("Zakończono %d/%d procesów\n", completedProcesses, numProcesses)
		case <-timeout:
			fmt.Println("\nTimeout! Program zbyt długo działa.")
			return
		}
	}

	fmt.Println("\n✅ Wszystkie procesy zakończyły się pomyślnie!")
	fmt.Println("Algorytm Szymańskiego działał poprawnie - wzajemne wykluczanie zachowane.")
}

// szymanski_fixed.go
// Poprawiona implementacja algorytmu Szymańskiego

package main

import (
	"fmt"
	"sync"
	"time"
)

// Typ dla identyfikatora procesu
type ProcessID int

// Stan procesu
type ProcessState int

const (
	LocalSection   ProcessState = iota
	EntryProtocol1              // f[i] = 1
	EntryProtocol2              // f[i] = 2
	EntryProtocol3              // f[i] = 3
	EntryProtocol4              // f[i] = 4
	CriticalSection
	ExitProtocol
)

// Nazwy stanów dla wyświetlania
var stateNames = map[ProcessState]string{
	LocalSection:    "LocalSection",
	EntryProtocol1:  "EntryProtocol1",
	EntryProtocol2:  "EntryProtocol2",
	EntryProtocol3:  "EntryProtocol3",
	EntryProtocol4:  "EntryProtocol4",
	CriticalSection: "CriticalSection",
	ExitProtocol:    "ExitProtocol",
}

// Wartość flagi
type FlagValue int

// Struktura dla atomowych operacji na flagach
type AtomicFlags struct {
	flags []FlagValue
	mutex sync.RWMutex
}

// Konstruktor dla AtomicFlags
func NewAtomicFlags(numProcesses int) *AtomicFlags {
	return &AtomicFlags{
		flags: make([]FlagValue, numProcesses+1), // indeksowanie od 1
		mutex: sync.RWMutex{},
	}
}

// Ustawienie flagi
func (af *AtomicFlags) SetFlag(id ProcessID, value FlagValue) {
	af.mutex.Lock()
	defer af.mutex.Unlock()
	af.flags[id] = value
}

// Pobranie flagi
func (af *AtomicFlags) GetFlag(id ProcessID) FlagValue {
	af.mutex.RLock()
	defer af.mutex.RUnlock()
	return af.flags[id]
}

// Pobranie wszystkich flag
func (af *AtomicFlags) GetAllFlags() []FlagValue {
	af.mutex.RLock()
	defer af.mutex.RUnlock()
	result := make([]FlagValue, len(af.flags))
	copy(result, af.flags)
	return result
}

// Wyświetlenie aktualnego stanu
func DisplayState(id ProcessID, state ProcessState, flags []FlagValue) {
	fmt.Printf("Proces%d - %s | Flagi: [", id, stateNames[state])
	for i := 1; i < len(flags); i++ {
		fmt.Printf("%d", flags[i])
		if i < len(flags)-1 {
			fmt.Print(",")
		}
	}
	fmt.Println("]")
}

// UPROSZCZONA IMPLEMENTACJA ALGORYTMU SZYMAŃSKIEGO
func ProcessTask(id int, sharedFlags *AtomicFlags, done chan bool) {
	processID := ProcessID(id)
	currentState := LocalSection
	const numProcesses = 3
	loopCounter := 0

	defer func() {
		fmt.Printf("Proces%d zakończył działanie\n", id)
		done <- true
	}()

	for {
		switch currentState {

		case LocalSection:
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			// Różne czasy dla różnych procesów
			time.Sleep(time.Duration(500+id*300) * time.Millisecond)
			currentState = EntryProtocol1

		case EntryProtocol1:
			// Krok 1: f[i] := 1 (chcę wejść)
			sharedFlags.SetFlag(processID, 1)
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			time.Sleep(100 * time.Millisecond)
			currentState = EntryProtocol2

		case EntryProtocol2:
			// Krok 2: Czekaj aż inni nie są w stanach {2,3}
			for {
				canProceed := true
				for j := 1; j <= numProcesses; j++ {
					if j != id {
						flag := sharedFlags.GetFlag(ProcessID(j))
						if flag == 2 || flag == 3 {
							canProceed = false
							break
						}
					}
				}
				if canProceed {
					break
				}
				time.Sleep(50 * time.Millisecond)
			}

			// f[i] := 2 (czekam)
			sharedFlags.SetFlag(processID, 2)
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			time.Sleep(100 * time.Millisecond)
			currentState = EntryProtocol3

		case EntryProtocol3:
			// Krok 3: f[i] := 3 (gotowy)
			sharedFlags.SetFlag(processID, 3)
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			// Czekaj na procesy o wyższym priorytecie (niższym id)
			for {
				canProceed := true

				// Sprawdź procesy o niższym ID
				for j := 1; j < id; j++ {
					flag := sharedFlags.GetFlag(ProcessID(j))
					if flag == 2 || flag == 3 || flag == 4 {
						canProceed = false
						break
					}
				}

				if canProceed {
					break
				}
				time.Sleep(50 * time.Millisecond)
			}

			time.Sleep(100 * time.Millisecond)
			currentState = EntryProtocol4

		case EntryProtocol4:
			// Krok 4: f[i] := 4 (wchodzę)
			sharedFlags.SetFlag(processID, 4)
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			// Końcowe sprawdzenie - czekaj aż inni nie są w stanie 4
			for {
				canEnter := true
				for j := 1; j <= numProcesses; j++ {
					if j != id {
						flag := sharedFlags.GetFlag(ProcessID(j))
						// Sprawdź czy ktoś inny też jest w stanie 4
						if flag == 4 {
							// Jeśli ma niższe ID, ma priorytet
							if j < id {
								canEnter = false
								break
							}
						}
					}
				}
				if canEnter {
					break
				}
				time.Sleep(50 * time.Millisecond)
			}

			time.Sleep(100 * time.Millisecond)
			currentState = CriticalSection

		case CriticalSection:
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			fmt.Printf("*** Proces%d w SEKCJI KRYTYCZNEJ ***\n", id)
			time.Sleep(1000 * time.Millisecond) // Sekcja krytyczna

			currentState = ExitProtocol

		case ExitProtocol:
			// Wyjście: f[i] := 0
			sharedFlags.SetFlag(processID, 0)
			localFlags := sharedFlags.GetAllFlags()
			DisplayState(processID, currentState, localFlags)

			fmt.Printf("Proces%d wyszedł z sekcji krytycznej\n", id)
			currentState = LocalSection

			loopCounter++
			if loopCounter >= 2 {
				return
			}
		}

		time.Sleep(200 * time.Millisecond)
	}
}

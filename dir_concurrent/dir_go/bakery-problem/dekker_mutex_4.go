package main

import (
	"fmt"
	"math/rand"
	"sort"
	"sync"
	"sync/atomic"
	"time"
)

const (
	LOCAL_SECTION = iota
	ENTRY_PROTOCOL
	CRITICAL_SECTION
	EXIT_PROTOCOL
)

var stateNames = []string{"LOCAL_SECTION", "ENTRY_PROTOCOL", "CRITICAL_SECTION", "EXIT_PROTOCOL"}

const (
	NrOfProcesses = 2 // Dekker's algorithm is for 2 processes only
	MinSteps      = 50
	MaxSteps      = 100
	MinDelay      = 10 * time.Millisecond
	MaxDelay      = 50 * time.Millisecond
)

type TraceEntry struct {
	TimeStamp time.Duration
	ID        int
	X         int
	Y         int
	Symbol    rune
}

// Dekker's Algorithm Variables
// Using atomic operations to ensure proper memory synchronization
type DekkerMutex struct {
	want [2]int32 // Array of atomic integers (0 = false, 1 = true)
	turn int32    // Atomic integer for turn (0 or 1)
}

func NewDekkerMutex() *DekkerMutex {
	return &DekkerMutex{
		want: [2]int32{0, 0}, // Both processes initially don't want to enter
		turn: 0,              // Process 0 starts with turn
	}
}

func (dm *DekkerMutex) setWant(processID int, value bool) {
	if value {
		atomic.StoreInt32(&dm.want[processID], 1)
	} else {
		atomic.StoreInt32(&dm.want[processID], 0)
	}
}

func (dm *DekkerMutex) getWant(processID int) bool {
	return atomic.LoadInt32(&dm.want[processID]) == 1
}

func (dm *DekkerMutex) getTurn() int {
	return int(atomic.LoadInt32(&dm.turn))
}

func (dm *DekkerMutex) setTurn(processID int) {
	atomic.StoreInt32(&dm.turn, int32(processID))
}

func (dm *DekkerMutex) Lock(processID int) {
	otherProcess := 1 - processID // If processID=0 then other=1, if processID=1 then other=0
	
	// Step 1: Indicate that we want to enter critical section
	dm.setWant(processID, true)
	
	// Step 2: Wait while the other process wants to enter AND it's their turn
	for dm.getWant(otherProcess) {
		if dm.getTurn() == otherProcess {
			// It's the other process's turn, so we back off
			dm.setWant(processID, false)
			
			// Wait until it's our turn
			for dm.getTurn() == otherProcess {
				time.Sleep(time.Millisecond)
			}
			
			// Now it's our turn, indicate we want to enter again
			dm.setWant(processID, true)
		}
		
		// Small delay to prevent tight spinning
		time.Sleep(time.Millisecond)
	}
}

func (dm *DekkerMutex) Unlock(processID int) {
	otherProcess := 1 - processID
	
	// Step 1: Give turn to the other process
	dm.setTurn(otherProcess)
	
	// Step 2: Indicate we no longer want to enter critical section
	dm.setWant(processID, false)
}

type Process struct {
	id        int
	symbol    rune
	mutex     *DekkerMutex
	traces    []TraceEntry
	startTime time.Time
}

func NewProcess(id int, symbol rune, mutex *DekkerMutex, startTime time.Time) *Process {
	return &Process{
		id:        id,
		symbol:    symbol,
		mutex:     mutex,
		traces:    make([]TraceEntry, 0),
		startTime: startTime,
	}
}

func (p *Process) addTrace(state int) {
	timestamp := time.Since(p.startTime)
	trace := TraceEntry{
		TimeStamp: timestamp,
		ID:        p.id,
		X:         p.id,
		Y:         state,
		Symbol:    p.symbol,
	}
	p.traces = append(p.traces, trace)
}

func (p *Process) run(wg *sync.WaitGroup, traceChan chan<- []TraceEntry) {
	defer wg.Done()
	
	// Create process-specific random generator
	rng := rand.New(rand.NewSource(time.Now().UnixNano() + int64(p.id*1000)))
	nrOfSteps := MinSteps + rng.Intn(MaxSteps-MinSteps)
	
	// Initial state
	p.addTrace(LOCAL_SECTION)
	
	for step := 0; step < nrOfSteps/4; step++ {
		// LOCAL_SECTION
		delay := MinDelay + time.Duration(rng.Float64()*float64(MaxDelay-MinDelay))
		time.Sleep(delay)
		
		// ENTRY_PROTOCOL
		p.addTrace(ENTRY_PROTOCOL)
		
		// Dekker's algorithm entry protocol
		p.mutex.Lock(p.id)
		
		// CRITICAL_SECTION
		p.addTrace(CRITICAL_SECTION)
		
		// Critical section work
		delay = MinDelay + time.Duration(rng.Float64()*float64(MaxDelay-MinDelay))
		time.Sleep(delay)
		
		// EXIT_PROTOCOL
		p.addTrace(EXIT_PROTOCOL)
		
		// Dekker's algorithm exit protocol
		p.mutex.Unlock(p.id)
		
		// Back to LOCAL_SECTION
		p.addTrace(LOCAL_SECTION)
	}
	
	// Send traces to channel
	traceChan <- p.traces
}

func main() {
	startTime := time.Now()
	mutex := NewDekkerMutex()
	
	var wg sync.WaitGroup
	traceChan := make(chan []TraceEntry, NrOfProcesses)
	
	// Start both processes
	for i := 0; i < NrOfProcesses; i++ {
		symbol := rune('A' + i)
		process := NewProcess(i, symbol, mutex, startTime)
		wg.Add(1)
		go process.run(&wg, traceChan)
	}
	
	// Wait for both processes to complete
	wg.Wait()
	close(traceChan)
	
	// Collect all traces
	var allTraces []TraceEntry
	for traces := range traceChan {
		allTraces = append(allTraces, traces...)
	}
	
	// Sort traces by timestamp
	sort.Slice(allTraces, func(i, j int) bool {
		return allTraces[i].TimeStamp < allTraces[j].TimeStamp
	})
	
	// Print traces in the format expected by display script
	for _, trace := range allTraces {
		fmt.Printf("%.9f %d %d %d %c\n",
			trace.TimeStamp.Seconds(),
			trace.ID,
			trace.X,
			trace.Y,
			trace.Symbol)
	}
	
	// Print the header line with parameters
	fmt.Printf("-1 %d %d %d ", NrOfProcesses, NrOfProcesses, len(stateNames))
	for _, state := range stateNames {
		fmt.Printf("%s;", state)
	}
	fmt.Printf("DEKKER_ALGORITHM;\n")
}

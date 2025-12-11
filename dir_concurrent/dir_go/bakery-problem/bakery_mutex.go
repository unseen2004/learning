package main

import (
	"fmt"
	"math/rand"
	"sort"
	"sync"
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
	NrOfProcesses = 15
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

type BakeryMutex struct {
	tickets  []int
	choosing []bool
	mutex    sync.RWMutex
}

func NewBakeryMutex(n int) *BakeryMutex {
	return &BakeryMutex{
		tickets:  make([]int, n),
		choosing: make([]bool, n),
	}
}

func (bm *BakeryMutex) getMaxTicket() int {
	bm.mutex.RLock()
	defer bm.mutex.RUnlock()
	
	max := 0
	for _, ticket := range bm.tickets {
		if ticket > max {
			max = ticket
		}
	}
	return max
}

func (bm *BakeryMutex) setChoosing(id int, value bool) {
	bm.mutex.Lock()
	defer bm.mutex.Unlock()
	bm.choosing[id] = value
}

func (bm *BakeryMutex) isChoosing(id int) bool {
	bm.mutex.RLock()
	defer bm.mutex.RUnlock()
	return bm.choosing[id]
}

func (bm *BakeryMutex) setTicket(id, ticket int) {
	bm.mutex.Lock()
	defer bm.mutex.Unlock()
	bm.tickets[id] = ticket
}

func (bm *BakeryMutex) getTicket(id int) int {
	bm.mutex.RLock()
	defer bm.mutex.RUnlock()
	return bm.tickets[id]
}

func (bm *BakeryMutex) hasPriority(myID, otherID int) bool {
	bm.mutex.RLock()
	defer bm.mutex.RUnlock()
	
	myTicket := bm.tickets[myID]
	otherTicket := bm.tickets[otherID]
	
	if myTicket < otherTicket {
		return true
	} else if myTicket == otherTicket {
		return myID < otherID
	}
	return false
}

func (bm *BakeryMutex) Lock(id int) {
	// Step 1: Signal that we're choosing a ticket
	bm.setChoosing(id, true)
	
	// Step 2: Get a new ticket (one higher than current maximum)
	maxTicket := bm.getMaxTicket()
	bm.setTicket(id, maxTicket+1)
	
	// Step 3: Finished choosing
	bm.setChoosing(id, false)
	
	// Step 4: Wait for our turn
	for otherID := 0; otherID < len(bm.tickets); otherID++ {
		if otherID != id {
			// Wait until other process finishes choosing
			for bm.isChoosing(otherID) {
				time.Sleep(time.Millisecond)
			}
			
			// Wait until we have priority or other process has no ticket
			for bm.getTicket(otherID) != 0 && !bm.hasPriority(id, otherID) {
				time.Sleep(time.Millisecond)
			}
		}
	}
}

func (bm *BakeryMutex) Unlock(id int) {
	// Release our ticket
	bm.setTicket(id, 0)
}

type Process struct {
	id           int
	symbol       rune
	mutex        *BakeryMutex
	traces       []TraceEntry
	startTime    time.Time
	maxTicket    int
	maxTicketMux sync.Mutex
}

func NewProcess(id int, symbol rune, mutex *BakeryMutex, startTime time.Time) *Process {
	return &Process{
		id:        id,
		symbol:    symbol,
		mutex:     mutex,
		traces:    make([]TraceEntry, 0),
		startTime: startTime,
		maxTicket: 0,
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

func (p *Process) updateMaxTicket(ticket int) {
	p.maxTicketMux.Lock()
	defer p.maxTicketMux.Unlock()
	if ticket > p.maxTicket {
		p.maxTicket = ticket
	}
}

func (p *Process) getMaxTicket() int {
	p.maxTicketMux.Lock()
	defer p.maxTicketMux.Unlock()
	return p.maxTicket
}

func (p *Process) run(wg *sync.WaitGroup, traceChan chan<- []TraceEntry, maxTicketChan chan<- int) {
	defer wg.Done()
	
	rng := rand.New(rand.NewSource(time.Now().UnixNano() + int64(p.id)))
	nrOfSteps := MinSteps + rng.Intn(MaxSteps-MinSteps)
	
	// Initial state
	p.addTrace(LOCAL_SECTION)
	
	for step := 0; step < nrOfSteps/4; step++ {
		// LOCAL_SECTION
		delay := MinDelay + time.Duration(rng.Float64()*float64(MaxDelay-MinDelay))
		time.Sleep(delay)
		
		// ENTRY_PROTOCOL
		p.addTrace(ENTRY_PROTOCOL)
		
		// Bakery algorithm entry protocol
		p.mutex.Lock(p.id)
		
		// Track the maximum ticket used by this process
		currentTicket := p.mutex.getTicket(p.id)
		p.updateMaxTicket(currentTicket)
		
		// CRITICAL_SECTION
		p.addTrace(CRITICAL_SECTION)
		
		// Critical section work
		delay = MinDelay + time.Duration(rng.Float64()*float64(MaxDelay-MinDelay))
		time.Sleep(delay)
		
		// EXIT_PROTOCOL
		p.addTrace(EXIT_PROTOCOL)
		
		// Bakery algorithm exit protocol
		p.mutex.Unlock(p.id)
		
		// Back to LOCAL_SECTION
		p.addTrace(LOCAL_SECTION)
	}
	
	// Send traces and max ticket to channels
	traceChan <- p.traces
	maxTicketChan <- p.getMaxTicket()
}

func main() {
	startTime := time.Now()
	mutex := NewBakeryMutex(NrOfProcesses)
	
	var wg sync.WaitGroup
	traceChan := make(chan []TraceEntry, NrOfProcesses)
	maxTicketChan := make(chan int, NrOfProcesses)
	
	// Start all processes
	for i := 0; i < NrOfProcesses; i++ {
		symbol := rune('A' + i)
		process := NewProcess(i, symbol, mutex, startTime)
		wg.Add(1)
		go process.run(&wg, traceChan, maxTicketChan)
	}
	
	// Wait for all processes to complete
	wg.Wait()
	close(traceChan)
	close(maxTicketChan)
	
	// Collect all traces
	var allTraces []TraceEntry
	for traces := range traceChan {
		allTraces = append(allTraces, traces...)
	}
	
	// Find maximum ticket used
	maxTicket := 0
	for ticket := range maxTicketChan {
		if ticket > maxTicket {
			maxTicket = ticket
		}
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
	fmt.Printf("MAX_TICKET=%d;\n", maxTicket)
}

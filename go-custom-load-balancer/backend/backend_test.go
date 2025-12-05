package backend

import (
	"net/url"
	"sync"
	"testing"
)

func TestBackendAliveState(t *testing.T) {
	u, _ := url.Parse("http://localhost:9001")
	b := NewBackend(u)

	// Should be alive initially
	if !b.IsAlive() {
		t.Error("Backend should be alive initially")
	}

	// Set to dead
	b.SetAlive(false)
	if b.IsAlive() {
		t.Error("Backend should be dead after SetAlive(false)")
	}

	// Set back to alive
	b.SetAlive(true)
	if !b.IsAlive() {
		t.Error("Backend should be alive after SetAlive(true)")
	}
}

func TestBackendConcurrentAliveState(t *testing.T) {
	u, _ := url.Parse("http://localhost:9001")
	b := NewBackend(u)

	var wg sync.WaitGroup
	iterations := 1000

	// Concurrent writes
	wg.Add(2)
	go func() {
		defer wg.Done()
		for i := 0; i < iterations; i++ {
			b.SetAlive(true)
		}
	}()
	go func() {
		defer wg.Done()
		for i := 0; i < iterations; i++ {
			b.SetAlive(false)
		}
	}()

	// Concurrent reads
	wg.Add(1)
	go func() {
		defer wg.Done()
		for i := 0; i < iterations; i++ {
			_ = b.IsAlive()
		}
	}()

	wg.Wait()
	// Test passes if no race condition occurs
}

func TestBackendConnectionCounting(t *testing.T) {
	u, _ := url.Parse("http://localhost:9001")
	b := NewBackend(u)

	// Should be 0 initially
	if b.GetActiveConns() != 0 {
		t.Errorf("Expected 0 connections, got %d", b.GetActiveConns())
	}

	// Increment
	b.IncrementConns()
	if b.GetActiveConns() != 1 {
		t.Errorf("Expected 1 connection, got %d", b.GetActiveConns())
	}

	// Increment more
	b.IncrementConns()
	b.IncrementConns()
	if b.GetActiveConns() != 3 {
		t.Errorf("Expected 3 connections, got %d", b.GetActiveConns())
	}

	// Decrement
	b.DecrementConns()
	if b.GetActiveConns() != 2 {
		t.Errorf("Expected 2 connections, got %d", b.GetActiveConns())
	}
}

func TestBackendConcurrentConnectionCounting(t *testing.T) {
	u, _ := url.Parse("http://localhost:9001")
	b := NewBackend(u)

	var wg sync.WaitGroup
	iterations := 1000

	// Concurrent increments
	wg.Add(1)
	go func() {
		defer wg.Done()
		for i := 0; i < iterations; i++ {
			b.IncrementConns()
		}
	}()

	// Concurrent decrements
	wg.Add(1)
	go func() {
		defer wg.Done()
		for i := 0; i < iterations; i++ {
			b.DecrementConns()
		}
	}()

	wg.Wait()

	// After equal increments and decrements, should be 0
	if b.GetActiveConns() != 0 {
		t.Errorf("Expected 0 connections after equal inc/dec, got %d", b.GetActiveConns())
	}
}

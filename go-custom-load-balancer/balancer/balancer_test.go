package balancer

import (
	"fmt"
	"net/url"
	"testing"

	"github.com/maks/go-custom-load-balancer/backend"
	"github.com/maks/go-custom-load-balancer/pool"
)

func createTestPool(n int) *pool.ServerPool {
	p := pool.NewServerPool()
	for i := 0; i < n; i++ {
		u, _ := url.Parse(fmt.Sprintf("http://localhost:%d", 9001+i))
		p.AddBackend(backend.NewBackend(u))
	}
	return p
}

func TestRoundRobinRotation(t *testing.T) {
	p := pool.NewServerPool()

	u1, _ := url.Parse("http://localhost:9001")
	u2, _ := url.Parse("http://localhost:9002")
	u3, _ := url.Parse("http://localhost:9003")

	b1 := backend.NewBackend(u1)
	b2 := backend.NewBackend(u2)
	b3 := backend.NewBackend(u3)

	p.AddBackend(b1)
	p.AddBackend(b2)
	p.AddBackend(b3)

	rr := &RoundRobin{}

	// Get 6 backends - should cycle through twice
	results := make([]string, 6)
	for i := 0; i < 6; i++ {
		b := rr.GetNextBackend(p)
		if b == nil {
			t.Fatal("GetNextBackend returned nil")
		}
		results[i] = b.URL.String()
	}

	// Verify round robin pattern (each backend appears twice)
	counts := make(map[string]int)
	for _, r := range results {
		counts[r]++
	}

	if counts["http://localhost:9001"] != 2 ||
		counts["http://localhost:9002"] != 2 ||
		counts["http://localhost:9003"] != 2 {
		t.Errorf("Expected each backend to be selected 2 times, got %v", counts)
	}
}

func TestRoundRobinSkipsDeadBackends(t *testing.T) {
	p := pool.NewServerPool()

	u1, _ := url.Parse("http://localhost:9001")
	u2, _ := url.Parse("http://localhost:9002")
	u3, _ := url.Parse("http://localhost:9003")

	b1 := backend.NewBackend(u1)
	b2 := backend.NewBackend(u2)
	b3 := backend.NewBackend(u3)

	// Mark backend 2 as dead
	b2.SetAlive(false)

	p.AddBackend(b1)
	p.AddBackend(b2)
	p.AddBackend(b3)

	rr := &RoundRobin{}

	// Get 10 backends - dead one should never be returned
	for i := 0; i < 10; i++ {
		b := rr.GetNextBackend(p)
		if b == nil {
			t.Fatal("GetNextBackend returned nil")
		}
		if b.URL.String() == "http://localhost:9002" {
			t.Error("Dead backend should not be returned")
		}
	}
}

func TestLeastConnectionsSelection(t *testing.T) {
	p := pool.NewServerPool()

	u1, _ := url.Parse("http://localhost:9001")
	u2, _ := url.Parse("http://localhost:9002")
	u3, _ := url.Parse("http://localhost:9003")

	b1 := backend.NewBackend(u1)
	b2 := backend.NewBackend(u2)
	b3 := backend.NewBackend(u3)

	// Simulate different connection counts
	b1.IncrementConns() // 1
	b1.IncrementConns() // 2
	b2.IncrementConns() // 1
	// b3 has 0

	p.AddBackend(b1)
	p.AddBackend(b2)
	p.AddBackend(b3)

	lc := &LeastConnections{}

	// Should always return b3 (has 0 connections)
	b := lc.GetNextBackend(p)
	if b.URL.String() != "http://localhost:9003" {
		t.Errorf("Expected backend with 0 connections (9003), got %s", b.URL.String())
	}

	// Increment b3's connections
	b3.IncrementConns() // b3 now has 1, b2 has 1, b1 has 2
	b3.IncrementConns() // b3 now has 2, b2 has 1, b1 has 2

	// Now should return b2 (has 1 connection, lowest)
	b = lc.GetNextBackend(p)
	if b.URL.String() != "http://localhost:9002" {
		t.Errorf("Expected backend with least connections (9002), got %s", b.URL.String())
	}
}

func TestLeastConnectionsSkipsDeadBackends(t *testing.T) {
	p := pool.NewServerPool()

	u1, _ := url.Parse("http://localhost:9001")
	u2, _ := url.Parse("http://localhost:9002")

	b1 := backend.NewBackend(u1)
	b2 := backend.NewBackend(u2)

	// b2 has 0 connections but is dead
	b2.SetAlive(false)
	b1.IncrementConns() // b1 has 1 connection

	p.AddBackend(b1)
	p.AddBackend(b2)

	lc := &LeastConnections{}

	// Should return b1 (b2 is dead)
	b := lc.GetNextBackend(p)
	if b.URL.String() != "http://localhost:9001" {
		t.Errorf("Expected alive backend (9001), got %s", b.URL.String())
	}
}

func TestEmptyPool(t *testing.T) {
	p := pool.NewServerPool()

	rr := &RoundRobin{}
	lc := &LeastConnections{}

	if rr.GetNextBackend(p) != nil {
		t.Error("RoundRobin should return nil for empty pool")
	}

	if lc.GetNextBackend(p) != nil {
		t.Error("LeastConnections should return nil for empty pool")
	}
}

func TestAllDeadBackends(t *testing.T) {
	p := pool.NewServerPool()

	u1, _ := url.Parse("http://localhost:9001")
	b1 := backend.NewBackend(u1)
	b1.SetAlive(false)
	p.AddBackend(b1)

	rr := &RoundRobin{}
	lc := &LeastConnections{}

	if rr.GetNextBackend(p) != nil {
		t.Error("RoundRobin should return nil when all backends are dead")
	}

	if lc.GetNextBackend(p) != nil {
		t.Error("LeastConnections should return nil when all backends are dead")
	}
}

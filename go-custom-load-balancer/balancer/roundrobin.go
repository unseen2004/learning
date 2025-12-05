package balancer

import (
	"github.com/maks/go-custom-load-balancer/backend"
	"github.com/maks/go-custom-load-balancer/pool"
)

// RoundRobin implements the round robin load balancing algorithm
type RoundRobin struct{}

// Name returns the algorithm name
func (r *RoundRobin) Name() string {
	return "Round Robin"
}

// GetNextBackend returns the next available backend in round robin fashion
func (r *RoundRobin) GetNextBackend(p *pool.ServerPool) *backend.Backend {
	backends := p.GetAllBackends()
	backendCount := len(backends)
	
	if backendCount == 0 {
		return nil
	}
	
	// Try to find an alive backend, starting from the next index
	// We'll try up to backendCount times to find an alive one
	next := p.NextIndex()
	
	for i := 0; i < backendCount; i++ {
		idx := (int(next) + i) % backendCount
		if backends[idx].IsAlive() {
			return backends[idx]
		}
	}
	
	// No alive backends found
	return nil
}

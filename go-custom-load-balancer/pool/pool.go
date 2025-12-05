package pool

import (
	"net/url"
	"sync"
	"sync/atomic"

	"github.com/maks/go-custom-load-balancer/backend"
)

// ServerPool holds information about reachable backends
type ServerPool struct {
	backends []*backend.Backend
	current  uint64 // Atomic counter for Round Robin
	mux      sync.RWMutex
}

// NewServerPool creates a new empty server pool
func NewServerPool() *ServerPool {
	return &ServerPool{
		backends: make([]*backend.Backend, 0),
	}
}

// AddBackend adds a new backend to the pool
func (s *ServerPool) AddBackend(b *backend.Backend) {
	s.mux.Lock()
	defer s.mux.Unlock()
	s.backends = append(s.backends, b)
}

// GetAllBackends returns all backends (for health checking)
func (s *ServerPool) GetAllBackends() []*backend.Backend {
	s.mux.RLock()
	defer s.mux.RUnlock()
	// Return a copy to avoid race conditions
	result := make([]*backend.Backend, len(s.backends))
	copy(result, s.backends)
	return result
}

// GetAliveBackends returns only the healthy backends
func (s *ServerPool) GetAliveBackends() []*backend.Backend {
	s.mux.RLock()
	defer s.mux.RUnlock()
	
	alive := make([]*backend.Backend, 0)
	for _, b := range s.backends {
		if b.IsAlive() {
			alive = append(alive, b)
		}
	}
	return alive
}

// BackendCount returns the total number of backends
func (s *ServerPool) BackendCount() int {
	s.mux.RLock()
	defer s.mux.RUnlock()
	return len(s.backends)
}

// AliveCount returns the number of alive backends
func (s *ServerPool) AliveCount() int {
	s.mux.RLock()
	defer s.mux.RUnlock()
	
	count := 0
	for _, b := range s.backends {
		if b.IsAlive() {
			count++
		}
	}
	return count
}

// NextIndex atomically increments and returns the next index (for Round Robin)
func (s *ServerPool) NextIndex() uint64 {
	return atomic.AddUint64(&s.current, 1)
}

// MarkBackendStatus updates the alive status of a backend by its URL
func (s *ServerPool) MarkBackendStatus(backendURL *url.URL, alive bool) {
	s.mux.RLock()
	defer s.mux.RUnlock()
	
	for _, b := range s.backends {
		if b.URL.String() == backendURL.String() {
			b.SetAlive(alive)
			return
		}
	}
}

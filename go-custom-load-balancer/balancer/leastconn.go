package balancer

import (
	"github.com/maks/go-custom-load-balancer/backend"
	"github.com/maks/go-custom-load-balancer/pool"
)

// LeastConnections implements the least connections load balancing algorithm
type LeastConnections struct{}

// Name returns the algorithm name
func (l *LeastConnections) Name() string {
	return "Least Connections"
}

// GetNextBackend returns the backend with the fewest active connections
func (l *LeastConnections) GetNextBackend(p *pool.ServerPool) *backend.Backend {
	backends := p.GetAliveBackends()
	
	if len(backends) == 0 {
		return nil
	}
	
	// Find the backend with the minimum connections
	var selected *backend.Backend
	minConns := int64(-1)
	
	for _, b := range backends {
		conns := b.GetActiveConns()
		if minConns == -1 || conns < minConns {
			minConns = conns
			selected = b
		}
	}
	
	return selected
}

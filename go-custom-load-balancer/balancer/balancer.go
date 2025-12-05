package balancer

import (
	"github.com/maks/go-custom-load-balancer/backend"
	"github.com/maks/go-custom-load-balancer/pool"
)

// Balancer is the interface for load balancing algorithms
type Balancer interface {
	// GetNextBackend returns the next available backend based on the algorithm
	GetNextBackend(p *pool.ServerPool) *backend.Backend
	// Name returns the name of the algorithm
	Name() string
}

// GetBalancer returns a Balancer instance based on the algorithm name
func GetBalancer(algorithm string) Balancer {
	switch algorithm {
	case "leastconn", "leastconnections":
		return &LeastConnections{}
	case "roundrobin":
		fallthrough
	default:
		return &RoundRobin{}
	}
}

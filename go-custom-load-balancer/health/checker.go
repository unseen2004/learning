package health

import (
	"log"
	"net/http"
	"time"

	"github.com/maks/go-custom-load-balancer/pool"
)

// Checker performs periodic health checks on backends
type Checker struct {
	pool     *pool.ServerPool
	interval time.Duration
	timeout  time.Duration
}

// NewChecker creates a new health checker
func NewChecker(p *pool.ServerPool, interval, timeout time.Duration) *Checker {
	return &Checker{
		pool:     p,
		interval: interval,
		timeout:  timeout,
	}
}

// Run starts the health checking loop (should be run as a goroutine)
func (h *Checker) Run() {
	// Initial health check
	h.checkAllBackends()

	ticker := time.NewTicker(h.interval)
	defer ticker.Stop()

	for range ticker.C {
		h.checkAllBackends()
	}
}

// checkAllBackends checks the health of all backends in the pool
func (h *Checker) checkAllBackends() {
	for _, b := range h.pool.GetAllBackends() {
		alive := h.checkBackend(b.URL.String())
		wasAlive := b.IsAlive()
		b.SetAlive(alive)

		// Log state changes
		if alive && !wasAlive {
			log.Printf("[Health] Backend %s is now ALIVE", b.URL)
		} else if !alive && wasAlive {
			log.Printf("[Health] Backend %s is now DEAD", b.URL)
		}
	}
}

// checkBackend checks if a single backend is healthy
func (h *Checker) checkBackend(backendURL string) bool {
	client := http.Client{
		Timeout: h.timeout,
	}

	// Try the /health endpoint first
	resp, err := client.Get(backendURL + "/health")
	if err != nil {
		return false
	}
	defer resp.Body.Close()

	// Consider 2xx status codes as healthy
	return resp.StatusCode >= 200 && resp.StatusCode < 300
}

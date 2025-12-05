package backend

import (
	"net/http"
	"net/http/httputil"
	"net/url"
	"sync"
	"sync/atomic"
)

// Backend represents a single backend server in the pool
type Backend struct {
	URL          *url.URL
	Alive        bool
	mux          sync.RWMutex
	ReverseProxy *httputil.ReverseProxy
	activeConns  int64 // Atomic counter for Least Connections algorithm
}

// NewBackend creates a new Backend instance with a configured reverse proxy
func NewBackend(serverURL *url.URL) *Backend {
	proxy := httputil.NewSingleHostReverseProxy(serverURL)
	
	// Custom error handler to allow retry logic
	proxy.ErrorHandler = func(w http.ResponseWriter, r *http.Request, err error) {
		// Don't write anything here - let the caller handle retries
	}
	
	return &Backend{
		URL:          serverURL,
		Alive:        true, // Assume alive until health check proves otherwise
		ReverseProxy: proxy,
	}
}

// SetAlive sets the backend's alive status in a thread-safe manner
func (b *Backend) SetAlive(alive bool) {
	b.mux.Lock()
	defer b.mux.Unlock()
	b.Alive = alive
}

// IsAlive returns the backend's alive status in a thread-safe manner
func (b *Backend) IsAlive() bool {
	b.mux.RLock()
	defer b.mux.RUnlock()
	return b.Alive
}

// IncrementConns atomically increments the active connection count
func (b *Backend) IncrementConns() {
	atomic.AddInt64(&b.activeConns, 1)
}

// DecrementConns atomically decrements the active connection count
func (b *Backend) DecrementConns() {
	atomic.AddInt64(&b.activeConns, -1)
}

// GetActiveConns returns the current active connection count
func (b *Backend) GetActiveConns() int64 {
	return atomic.LoadInt64(&b.activeConns)
}

// String returns the backend URL as a string
func (b *Backend) String() string {
	return b.URL.String()
}

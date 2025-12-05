package proxy

import (
	"context"
	"log"
	"net"
	"net/http"
	"strings"
	"time"

	"github.com/maks/go-custom-load-balancer/backend"
	"github.com/maks/go-custom-load-balancer/balancer"
	"github.com/maks/go-custom-load-balancer/pool"
)

// Proxy is the load balancing HTTP handler
type Proxy struct {
	pool       *pool.ServerPool
	balancer   balancer.Balancer
	maxRetries int
}

// NewProxy creates a new load balancing proxy
func NewProxy(p *pool.ServerPool, b balancer.Balancer, maxRetries int) *Proxy {
	return &Proxy{
		pool:       p,
		balancer:   b,
		maxRetries: maxRetries,
	}
}

// ServeHTTP handles incoming requests and proxies them to backends
func (p *Proxy) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	// Track which backends we've already tried
	attempted := make(map[string]bool)

	for attempt := 0; attempt < p.maxRetries; attempt++ {
		// Get next backend
		b := p.getNextBackendExcluding(attempted)
		if b == nil {
			log.Printf("[Proxy] No available backends after %d attempts", attempt)
			http.Error(w, "Service Unavailable", http.StatusServiceUnavailable)
			return
		}

		// Mark this backend as attempted
		attempted[b.URL.String()] = true

		// Increment connection count for Least Connections algorithm
		b.IncrementConns()

		// Add/update X-Forwarded-For header
		clientIP := getClientIP(r)
		if prior := r.Header.Get("X-Forwarded-For"); prior != "" {
			clientIP = prior + ", " + clientIP
		}
		r.Header.Set("X-Forwarded-For", clientIP)

		// Add X-Forwarded-Host header
		r.Header.Set("X-Forwarded-Host", r.Host)

		// Try to proxy the request
		log.Printf("[Proxy] Attempting request to %s (attempt %d/%d)", b.URL, attempt+1, p.maxRetries)

		success := p.proxyRequest(b, w, r)

		// Decrement connection count
		b.DecrementConns()

		if success {
			return
		}

		// Mark backend as dead for failed request
		log.Printf("[Proxy] Backend %s failed, marking as dead", b.URL)
		p.pool.MarkBackendStatus(b.URL, false)
	}

	log.Printf("[Proxy] All retry attempts exhausted")
	http.Error(w, "Service Unavailable", http.StatusServiceUnavailable)
}

// getNextBackendExcluding returns the next backend that hasn't been attempted yet
func (p *Proxy) getNextBackendExcluding(attempted map[string]bool) *backend.Backend {
	// Try multiple times to get a backend that hasn't been attempted
	for i := 0; i < p.pool.BackendCount(); i++ {
		b := p.balancer.GetNextBackend(p.pool)
		if b == nil {
			return nil
		}
		if !attempted[b.URL.String()] {
			return b
		}
	}
	return nil
}

// proxyRequest proxies a single request to a backend
func (p *Proxy) proxyRequest(b *backend.Backend, w http.ResponseWriter, r *http.Request) bool {
	// Create a custom response writer to detect if we've written anything
	crw := &captureResponseWriter{
		ResponseWriter: w,
		statusCode:     0,
		written:        false,
	}

	// Create a context with timeout for the proxy request
	ctx, cancel := context.WithTimeout(r.Context(), 30*time.Second)
	defer cancel()
	r = r.WithContext(ctx)

	// Perform the proxy request
	b.ReverseProxy.ServeHTTP(crw, r)

	// Check if the request was successful
	// If nothing was written or we got an error status, consider it a failure
	if !crw.written {
		return false
	}

	return true
}

// captureResponseWriter wraps ResponseWriter to detect if anything was written
type captureResponseWriter struct {
	http.ResponseWriter
	statusCode int
	written    bool
}

func (c *captureResponseWriter) WriteHeader(code int) {
	c.statusCode = code
	c.written = true
	c.ResponseWriter.WriteHeader(code)
}

func (c *captureResponseWriter) Write(b []byte) (int, error) {
	c.written = true
	return c.ResponseWriter.Write(b)
}

// getClientIP extracts the client IP from the request
func getClientIP(r *http.Request) string {
	// Try to get IP from X-Real-IP header first
	ip := r.Header.Get("X-Real-IP")
	if ip != "" {
		return ip
	}

	// Try RemoteAddr
	ip, _, err := net.SplitHostPort(r.RemoteAddr)
	if err != nil {
		return strings.Split(r.RemoteAddr, ":")[0]
	}
	return ip
}

package main

import (
	"flag"
	"log"
	"net/http"
	"net/url"
	"strings"
	"time"

	"github.com/maks/go-custom-load-balancer/backend"
	"github.com/maks/go-custom-load-balancer/balancer"
	"github.com/maks/go-custom-load-balancer/health"
	"github.com/maks/go-custom-load-balancer/pool"
	"github.com/maks/go-custom-load-balancer/proxy"
)

func main() {
	// Parse command line flags
	port := flag.String("port", "8080", "Load balancer port")
	backends := flag.String("backends", "", "Comma-separated list of backend URLs (e.g., http://localhost:9001,http://localhost:9002)")
	algorithm := flag.String("algorithm", "roundrobin", "Load balancing algorithm: roundrobin or leastconn")
	healthInterval := flag.Duration("health-interval", 10*time.Second, "Health check interval")
	healthTimeout := flag.Duration("health-timeout", 2*time.Second, "Health check timeout")
	maxRetries := flag.Int("max-retries", 3, "Maximum retry attempts for failed requests")
	flag.Parse()

	// Validate backends
	if *backends == "" {
		log.Fatal("No backends specified. Use -backends flag to specify backend URLs.")
	}

	// Create server pool
	serverPool := pool.NewServerPool()

	// Parse and add backends
	backendURLs := strings.Split(*backends, ",")
	for _, rawURL := range backendURLs {
		rawURL = strings.TrimSpace(rawURL)
		if rawURL == "" {
			continue
		}

		parsedURL, err := url.Parse(rawURL)
		if err != nil {
			log.Fatalf("Invalid backend URL '%s': %v", rawURL, err)
		}

		b := backend.NewBackend(parsedURL)
		serverPool.AddBackend(b)
		log.Printf("[Init] Added backend: %s", parsedURL)
	}

	if serverPool.BackendCount() == 0 {
		log.Fatal("No valid backends configured")
	}

	// Get the appropriate balancer
	lb := balancer.GetBalancer(*algorithm)
	log.Printf("[Init] Using %s algorithm", lb.Name())

	// Start health checker
	healthChecker := health.NewChecker(serverPool, *healthInterval, *healthTimeout)
	go healthChecker.Run()
	log.Printf("[Init] Health checker started (interval: %s, timeout: %s)", *healthInterval, *healthTimeout)

	// Create proxy handler
	proxyHandler := proxy.NewProxy(serverPool, lb, *maxRetries)

	// Start the load balancer server
	server := &http.Server{
		Addr:         ":" + *port,
		Handler:      proxyHandler,
		ReadTimeout:  30 * time.Second,
		WriteTimeout: 30 * time.Second,
	}

	log.Printf("[Init] Load balancer started on :%s with %d backends", *port, serverPool.BackendCount())
	log.Fatal(server.ListenAndServe())
}

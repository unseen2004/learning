package main

import (
	"flag"
	"fmt"
	"log"
	"math/rand"
	"net/http"
	"time"
)

func main() {
	port := flag.Int("port", 9001, "Backend server port")
	slowMode := flag.Bool("slow", false, "Enable slow mode (simulates processing delay)")
	failRate := flag.Int("fail-rate", 0, "Percentage of requests to fail (0-100)")
	flag.Parse()

	serverName := fmt.Sprintf("Backend-%d", *port)

	// Health check endpoint
	http.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		fmt.Fprintf(w, "OK")
	})

	// Main handler
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		// Simulate random failures
		if *failRate > 0 && rand.Intn(100) < *failRate {
			log.Printf("[%s] Simulating failure", serverName)
			http.Error(w, "Internal Server Error", http.StatusInternalServerError)
			return
		}

		// Simulate processing time
		if *slowMode {
			delay := time.Duration(100+rand.Intn(400)) * time.Millisecond
			log.Printf("[%s] Processing request (delay: %v)", serverName, delay)
			time.Sleep(delay)
		}

		// Log incoming request details
		log.Printf("[%s] Received request: %s %s from %s (X-Forwarded-For: %s)",
			serverName,
			r.Method,
			r.URL.Path,
			r.RemoteAddr,
			r.Header.Get("X-Forwarded-For"),
		)

		// Send response
		w.Header().Set("Content-Type", "text/plain")
		w.Header().Set("X-Backend-Server", serverName)
		fmt.Fprintf(w, "Hello from %s!\n", serverName)
		fmt.Fprintf(w, "Request path: %s\n", r.URL.Path)
		fmt.Fprintf(w, "Your IP (X-Forwarded-For): %s\n", r.Header.Get("X-Forwarded-For"))
	})

	addr := fmt.Sprintf(":%d", *port)
	log.Printf("[%s] Starting on %s", serverName, addr)

	if *slowMode {
		log.Printf("[%s] Slow mode enabled (100-500ms delay per request)", serverName)
	}
	if *failRate > 0 {
		log.Printf("[%s] Fail rate: %d%%", serverName, *failRate)
	}

	log.Fatal(http.ListenAndServe(addr, nil))
}

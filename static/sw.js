"use strict";

#ifdef DEBUG
var cacheVersion = `OWOP_VERSION-dbg`;
#else
var cacheVersion = `OWOP_VERSION`;
#endif

var fileList =
`OWOP_SCRIPT_PATH
#include <static_files.txt>
OWOP_WASM_PATH`.split("\n");

async function sendMsg(msg) {
	var clients = await self.clients.matchAll() || [];
	for (var i = 0; i < clients.length; i++) {
		clients[i].postMessage(msg);
	}
}

self.addEventListener("install", function(e) {
	e.waitUntil(
		caches.open(cacheVersion)
		.then(function(cache) {
			return cache.addAll(fileList);
		})
		.then(function() {
			return self.skipWaiting();
		})
	);
});

self.addEventListener("activate", function(e) {
	e.waitUntil((async function() {
		console.log("[SW] Activating new service worker");
		await self.clients.claim();

		await sendMsg({t: "updateAvailable", v: cacheVersion});

		var keyList = await caches.keys();
		await Promise.all(keyList.map(function(key) {
			if (key !== cacheVersion) {
				return caches.delete(key);
			}
		}));
	})());
});

self.addEventListener("fetch", function(e) {
	var url = new URL(e.request.url);
	if (!e.request.url.startsWith(self.location.origin)
			|| e.request.method !== "GET"
			|| url.pathname.startsWith("/api/")) {
		return;
	}

	e.respondWith((async function() {
		var r = await caches.match(e.request);

		if (!r && url.pathname.slice(1).indexOf("/") === -1
				&& !fileList.includes(url.pathname)) {
			r = await caches.match("/index.html");
		}

		if (!r) {
			r = await fetch(e.request);
		}

		return r;
	})());
});

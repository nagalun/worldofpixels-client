"use strict";

var cacheVersion = "2";

var fileList = [
	"/index.html",
	"/owop.js",
	"/owop.wasm",
	"/favicon.ico",
	"/sfx/button.mp3",
	"/sfx/join.mp3",
	"/sfx/pixel.mp3",
	"/img/banner.gif",
	"/img/bg.png",
	"/img/planet.gif",
	"/img/app/owop-72.png",
	"/img/app/owop-96.png",
	"/img/app/owop-2x-144.png",
	"/img/app/owop-2x-192.png",
	"/img/app/owop-512.png"
];

self.addEventListener("install", function(e) {
	e.waitUntil(
		caches.open(cacheVersion)
		.then(function(cache) {
			return cache.addAll(fileList);
		})
	);
});

self.addEventListener("activate", function(e) {
	e.waitUntil(
		caches.keys()
		.then(function(keyList) {
			return Promise.all(keyList.map(function(key) {
				if (key !== cacheVersion) {
					return caches.delete(key);
				}
			}));
		})
	);
});

self.addEventListener("fetch", function(e) {
	e.respondWith(
		caches.match(e.request)
		.then(function(r) {
			if (r) {
				return r;
			}

			return fetch(e.request);
		})
	);
});

"use strict";

function initColorPicker(box, source) {
	var color = source.value;
	box.value = color;
	source.type = "hidden";
	var picker = new CP(box);
	box.className = picker.state.class + " " + source.className;
	box.style.backgroundColor = source.value;

	var cx = 0;
	var cy = 0;
	box.addEventListener("touchend", function(e) {
		e.preventDefault();
	}, true);

	picker.self.addEventListener("mousedown", function(e) {
		cx = e.offsetX;
		cy = e.offsetY;
	});

	picker.self.addEventListener("touchstart", function(e) {
		var b = picker.self.getBoundingClientRect();
		cx = e.targetTouches[0].clientX - b.x;
		cy = e.targetTouches[0].clientY - b.y;
	}, { passive: true });

	function closeHdlr(endEvt) {
		picker.self.addEventListener(endEvt, function(e) {
			var b = picker.self.getBoundingClientRect();
			if (cx < 0 || cy < 0 || cx > b.width || cy > b.height) {
				picker.exit();
			}
			e.preventDefault();
		});
	}

	closeHdlr("mouseup");
	closeHdlr("touchend");

	picker.on("fit", function() {
		/* align it to the top of the element */
		var b = box.getBoundingClientRect();
		this.self.style.top = "unset";
		this.self.style.bottom = window.innerHeight - b.top + "px";
		cx = 0;
		cy = 0;
	});

	window.addEventListener("keydown", function(e) {
		if (e.code === "Escape") {
			picker.exit();
		}
	});

	var init = true; // absolutely terrible
	picker.on("change", function(r, g, b, a) {
		if (init) {
			init = false;
			return;
		}

		var ncolor = this.color(r, g, b, a);
		source.value = ncolor;
		box.value = ncolor;
		box.style.backgroundColor = ncolor;
		source.dispatchEvent(new Event("change"));
	});
}

function showUpdateAvailableAlert() {
#ifdef DEBUG
#ifndef DISABLE_AUTO_REFRESH
	location.reload(true);
#endif
#endif
}

#ifdef DEBUG
#ifndef DISABLE_AUTO_REFRESH
function enableAutoRefreshClient() {
	var ws = null;
	var to = null;
	function conn() {
		ws = new WebSocket("ws://" + location.hostname + ":9005");
		ws.onmessage = function(m) {
			console.log(m.data);
			clearTimeout(to);
			to = setTimeout(function() {
				if (window.SW && window.SW.update) {
					window.SW.update();
				} else {
					location.reload(true);
				}
			}, 1500);
		};
	}

	try { conn(); } catch (e) { }
}

enableAutoRefreshClient();
#endif
#endif

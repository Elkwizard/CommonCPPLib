#pragma once

#include "../util/windows.hpp"
#include "../util/debug.hpp"
#include "../util/string.hpp"

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace window {
	class Point {
		public:
			int x, y;

			Point(int _x, int _y) {
				x = _x;
				y = _y;
			}
			
			Point() : Point(0, 0) { }
	};

	class Rect {
		public:
			int x, y, width, height;

			Rect() {
				x = 0;
				y = 0;
				width = 0;
				height = 0;
			}

			Rect(int _x, int _y, int _width, int _height) {
				x = _x;
				y = _y;
				width = _width;
				height = _height;
			}

			bool contains(Point p) {
				return p.x >= x && p.y >= y && p.x <= x + width && p.y <= y + height; 
			}
	};

	std::ostream& operator <<(std::ostream& out, const Rect& rect);

	using CallbackID = int;
	
	class Event {
		public:
			using Callback = std::function<void()>;

		private:
			std::unordered_map<CallbackID, Callback> callbacks; 
			int nextID = 0;

		public:
			Event() { }

			CallbackID operator ()(const Callback& fn) {
				CallbackID id = nextID++;
				callbacks.emplace(id, fn);
				return id;
			}

			void remove(CallbackID id) {
				callbacks.erase(id);
			}

			void run() {
				for (const auto& callback : callbacks)
					callback.second();
			}
	};
	
	class EventHandler {
		private:
			CallbackID id;
			Event& event;

		public:
			EventHandler(Event& _event, const Event::Callback& fn) : event(_event) {
				id = event(fn);
			}

			~EventHandler() {
				event.remove(id);
			}
	};

	class Window {
		private:
			HINSTANCE appInstance;
			int resizeBuffer = 0;
			
			void invalidateWindowCaches() {
				SetWindowPos(handle, HWND_TOP, location.x - resizeBuffer, location.y, location.width + resizeBuffer * 2, location.height + resizeBuffer, SWP_FRAMECHANGED);
			}
			
		public:
			Event onTimer { };
			Event onResize { };
			HWND handle;
			Rect location;
			Rect client;
			std::string title, icon;

			Window(std::string _title, Rect _location = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT }) {
				title = _title;

				std::wstring wtitle = util::widen(title);
				
				appInstance = GetModuleHandle(NULL);

				WNDCLASS windowClass = { };
				windowClass.lpfnWndProc = windowProcedure;
				windowClass.hInstance = appInstance;
				windowClass.lpszClassName = wtitle.c_str();

				RegisterClass(&windowClass);

				handle = CreateWindowEx(
					0, // optional styles
					wtitle.c_str(), wtitle.c_str(), // name
					WS_OVERLAPPEDWINDOW,
					_location.x, _location.y, _location.width, _location.height, // location
					
					NULL, NULL, appInstance, this
				);

				ShowWindow(handle, SW_SHOW);
				SetTimer(handle, 0, 1000 / 80 /* fps (vaguely) */, NULL);

				adjustSize();
				resizeBuffer = (location.width - client.width) / 2;
				
				if (_location.x == CW_USEDEFAULT) {
					adjustSize();
					move(location);
				} else move(_location);
			}

			Window(const std::string& title, const std::string& _icon, Rect location = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT })
			: Window(title, location) {
				setIcon(_icon);
			}

			void setTitle(const std::string& _title) {
				title = _title;
				SetWindowText(handle, util::widen(title).c_str());
			}

			void setIcon(const std::string& _icon) {
				HICON hIcon = (HICON)LoadImage(
					appInstance,
					util::widen(_icon).c_str(),
					IMAGE_ICON,
					32, 32,
					LR_DEFAULTCOLOR | LR_LOADFROMFILE
				);
				SendMessage(handle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				icon = _icon;
			}

			void run() {
				MSG msg { };
				while (GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			void move(Rect loc) {
				MoveWindow(handle, loc.x - resizeBuffer, loc.y, loc.width + resizeBuffer * 2, loc.height + resizeBuffer, true);
				adjustSize(true);
			}

			void hide() {
				ShowWindow(handle, SW_HIDE);
			}

			void excludeFromCaptures() {
				SetWindowDisplayAffinity(handle, WDA_EXCLUDEFROMCAPTURE);
				invalidateWindowCaches();
			}

			void show() {
				ShowWindow(handle, SW_SHOW);
			}

			void maximize() {
				ShowWindow(handle, SW_MAXIMIZE);
				adjustSize(true);
			}
			
			void removeTitleBar() {
				SetWindowLong(handle, GWL_STYLE, 0);
				invalidateWindowCaches();
				adjustSize(true);
			}

			void minimize() {
				ShowWindow(handle, SW_MINIMIZE);
			}

			void close() {
				PostQuitMessage(0);
			}

			bool focused() {
				return GetFocus() == handle;
			}

#ifdef VULKAN_HPP
			vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) {
				vk::Win32SurfaceCreateInfoKHR createInfo {
					.hinstance = appInstance,
					.hwnd = handle
				};

				return instance.createWin32SurfaceKHR(createInfo);
			}
#endif

		private:
			void adjustSize(bool callback = false) {
				RECT r { };
				GetWindowRect(handle, &r);
				r.left += resizeBuffer;
				r.right -= resizeBuffer;
				r.bottom -= resizeBuffer;
				
				bool resized = r.right - r.left != location.width || r.bottom - r.top != location.height;
				location.x = r.left;
				location.y = r.top;
				location.width = r.right - r.left;
				location.height = r.bottom - r.top;

				RECT c { };
				GetClientRect(handle, &c);
				client.x = c.left;
				client.y = c.top;
				client.width = c.right - c.left;
				client.height = c.bottom - c.top;
				
				if (resized || callback) {
					onResize.run();
				}
			}

			static LRESULT CALLBACK windowProcedure(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
				if (msg == WM_CREATE) {
					Window* l = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
					SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)l);
				}

				Window* w = reinterpret_cast<Window*>(GetWindowLongPtr(handle, GWLP_USERDATA));

				switch (msg) {
					case WM_CLOSE: {
						w->close();
					}; break;
					case WM_SIZE:
					case WM_MOVE: {
						w->adjustSize();
					}; break;
					case WM_TIMER: {
						w->onTimer.run();
					}; break;
				}

				return DefWindowProc(handle, msg, wParam, lParam);
			}
	};
	
	class Input {
		private:
			static constexpr int REPEAT_DELAY = 20;
			static constexpr int REPEAT_INTERVAL = 3;
			
			std::unique_ptr<EventHandler> callback;

		protected:
			std::unordered_map<std::string, int> keyMap;
			std::unordered_map<std::string, bool> keysDown;
			std::unordered_map<std::string, int> keyDownCounts;
			Window& w;
		
		public:
			Input(Window& _w, std::unordered_map<std::string, int> _keyMap) : w(_w) {
				keyMap = _keyMap;
				for (const auto& [key, id] : keyMap) {
					keyDownCounts.emplace(key, 0);
					keysDown.emplace(key, false);
				}
				
				// update
				callback = std::make_unique<EventHandler>(w.onTimer, [&]() {
					bool focused = w.focused();
					for (const auto& [key, id] : keyMap) {
						keysDown.at(key) = focused && GetAsyncKeyState(id);
						if (pressed(key)) keyDownCounts.at(key)++;
						else keyDownCounts.at(key) = 0;
					}
				});
			}

			bool pressed(const std::string& key) const {
				return keysDown.at(key);
			}

			bool justPressed(const std::string& key) const {
				return keyDownCounts.at(key) == 1;
			}
			
			bool pressedRepeated(const std::string& key) const {
				if (!pressed(key)) return false;
				int time = keyDownCounts.at(key);
				return time == 1 || (time >= REPEAT_DELAY && (time - REPEAT_DELAY) % REPEAT_INTERVAL == 0);
			}
	};

	class Mouse : public Input, public Point {
		public:
			enum Cursor { WAIT, POINTER, NONE };

		private:
			std::unordered_map<Cursor, LPWSTR> cursorNameMap {
				{ WAIT, IDC_WAIT },
				{ POINTER, IDC_ARROW }
			};

			std::unordered_map<LPWSTR, HCURSOR> cursorHandleMap { };

			Cursor displayedCursor = POINTER;
			Point lockPoint;
			bool locked = false;
			int timeFocused = 0;
			std::unique_ptr<EventHandler> callback;

		public:
			int movementX = 0, movementY = 0;
			Cursor cursor = POINTER;
			Mouse(Window& w) : Input(w, {
				{ "Left", VK_LBUTTON },
				{ "Middle", VK_MBUTTON },
				{ "Right", VK_RBUTTON },
			}) {
				callback = std::make_unique<EventHandler>(w.onTimer, [&]() {
					POINT mouse;
					
					GetCursorPos(&mouse);
					ScreenToClient(w.handle, &mouse);

					if (w.focused() && w.client.contains({ mouse.x, mouse.y })) {
						if (locked) {
							POINT newMouse { lockPoint.x, lockPoint.y };
							ClientToScreen(w.handle, &newMouse);
							SetCursorPos(newMouse.x, newMouse.y);
						}

						if (timeFocused > 0) {
							if (locked) {
								movementX = mouse.x - lockPoint.x;
								movementY = mouse.y - lockPoint.y;
							} else {
								movementX = mouse.x - x;
								movementY = mouse.y - y;
							}
						}

						x = mouse.x;
						y = mouse.y;

						if (cursor == NONE) SetCursor(NULL);
						else {
							LPWSTR id = cursorNameMap.at(cursor);
							if (!cursorHandleMap.count(id)) 
								cursorHandleMap.emplace(id, LoadCursor(NULL, id));
							SetCursor(cursorHandleMap.at(id));
						}

						timeFocused++;
					} else {
						timeFocused = 0;
						movementX = 0;
						movementY = 0;
					}
				});
			}

			void lock(Point p) {
				locked = true;
				lockPoint = p;
			}

			void unlock() {
				locked = false;
			}

	};

	class Keyboard : public Input {
		public:
			Keyboard(Window& w) : Input(w, {
				{ "Tab", VK_TAB }, { "Backspace", VK_BACK }, { "Enter", VK_RETURN }, { "Shift", VK_SHIFT }, 
				{ "Control", VK_CONTROL }, { "Alt", VK_MENU }, { "Escape", VK_ESCAPE }, { " ", VK_SPACE },
				{ "ArrowUp", VK_UP }, { "ArrowDown", VK_DOWN }, { "ArrowLeft", VK_LEFT }, { "ArrowRight", VK_RIGHT },
				{ "0", 0x30 }, { "1", 0x31 }, { "2", 0x32 }, { "3", 0x33 }, { "4", 0x34 }, { "5", 0x35 },
				{ "6", 0x36 }, { "7", 0x37 }, { "8", 0x38 }, { "9", 0x39 },
				{ "a", 0x41 }, { "b", 0x42 }, { "c", 0x43 }, { "d", 0x44 }, { "e", 0x45 }, { "f", 0x46 },
				{ "g", 0x47 }, { "h", 0x48 }, { "i", 0x49 }, { "j", 0x4A }, { "k", 0x4B }, { "l", 0x4C },
				{ "m", 0x4D }, { "n", 0x4E }, { "o", 0x4F }, { "p", 0x50 }, { "q", 0x51 }, { "r", 0x52 },
				{ "s", 0x53 }, { "t", 0x54 }, { "u", 0x55 }, { "v", 0x56 }, { "w", 0x57 }, { "x", 0x58 },
				{ "y", 0x59 }, { "z", 0x5A },
				{ "+", VK_ADD }, { "-", VK_SUBTRACT }, { "*", VK_MULTIPLY }, { "/", VK_DIVIDE },
				{ ",", VK_OEM_COMMA }, { ".", VK_OEM_PERIOD }
			}) { }
	};
}
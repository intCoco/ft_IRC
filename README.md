<h1 align="center">ft_irc</h1>

<p align="center">
	<b>💬 <i>A fully functional IRC (Internet Relay Chat) server implemented in C++.</i></b><br>
</p>

<p align="center">
  <img alt="Language" src="https://img.shields.io/badge/Language-C++-blue"/>
  <img alt="Status" src="https://img.shields.io/badge/Status-Completed-success"/>
  <img src="https://img.shields.io/badge/Grade-100%2F100-success"/>
  <img alt="42" src="https://img.shields.io/badge/School-42-black"/>
</p>

---

## 📑 Table of Contents

* [📖 Description](#-description)
* [🚀 Features](#-features)
* [▶️ Installation & Usage](#-installation--usage)
* [🧠 How It Works](#-how-it-works)
* [📂 Project Structure](#-project-structure)
* [🎯 Learning Objectives](#-learning-objectives)
* [🏆 Conclusion](#-conclusion)
* [👤 Authors](#-authors)

---

## 📖 Description

ft_irc is a network server project developed as part of the 42 curriculum. The goal is to implement a fully functional **Internet Relay Chat server** in C, capable of handling multiple clients, channels, and standard IRC commands.

It demonstrates a deep understanding of network programming, socket management, client-server communication, and protocol adherence. The server is fully POSIX compliant and handles real-time chat interactions efficiently.

---

## 🚀 Features

* Multi-client support
* Multiple channels per server
* Nickname management
* User authentication and registration per session
* Private and public messaging
* Channel operators and permissions
* Real-time message broadcast
* Standard IRC commands support

---

## ▶️ Installation & Usage

### Requirements:

* GCC / Clang
* Make
* Unix-based system (Linux / macOS)

### How to run the server:

1. Build the project:  
	```bash
	make
	```

2. Start the server:
	```bash
	./ircserv <port> <password>
	```

> Example:
> ```bash
> ./ircserv 6667 mysecretpassword
> ```

3. Connect with any IRC client using the same port and password.

---

## 🧠 How It Works

* The server uses **sockets and the select() system call** to handle multiple clients concurrently.
* Each client connection is managed in a non-blocking manner.
* The server parses standard IRC commands and maintains internal state for users, channels, and operator privileges.
* Messages are broadcasted to appropriate channels or users in real-time.

---

## 📂 Project Structure

```
ft_irc/
├── src/            # Source files
├── includes/       # Header files
├── Makefile
└── README.md
```

---

## 🎯 Learning Objectives

* Network programming fundamentals
* TCP socket management
* Client-server architecture
* Protocol implementation (IRC)
* Concurrent programming with select()
* Memory management in C
* Parsing and command handling

---

## 🏆 Conclusion

**ft_irc** is a comprehensive exploration of network programming and real-time communication. It demonstrates how to implement a robust server that follows an established protocol while efficiently managing multiple clients.

---

## 👤 Authors

*This project has been created as part of the 42 curriculum bychuchard and nihamdan*

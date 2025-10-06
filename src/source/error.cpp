#include "error.hpp"

/**
 * class Message
 */

MessageBuilder Message::of() {
	return {};
}

/**
 * class MessageSink
 */

MessageSink& MessageSink::instance() {
	static MessageSink sink;
	return sink;
}

void MessageSink::printer(const Printer& printer) {
	instance().m_printer = printer;
}

void MessageSink::feed(const Message& message) {
	instance().m_printer(message);
}

void MessageSink::disable() {
	instance().m_printer = [] (const Message&) noexcept -> void {};
}

/**
 * class MessageBuilder
 */
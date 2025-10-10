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
	MessageSink& sink = instance();

	sink.m_printer(message);

	if (!sink.failed) {
		sink.failed = message.error();
	}
}

void MessageSink::clear() {
	instance().failed = false;
}

bool MessageSink::error() {
	return instance().failed;
}


void MessageSink::disable() {
	instance().m_printer = [] (const Message&) noexcept -> void {};
}

/**
 * class MessageBuilder
 */
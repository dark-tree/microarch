#pragma once
#include <functional>
#include <list>

#include "token.hpp"

class MessageBuilder;

/**
 * This object represents a single (logical) message as it was constructed by the compiler,
 * a single message can contain one (or more) message nodes of varying severity that and with separate source
 * spans. For example to show a redeclaration error, and previous declaration location info.
 */
class Message : public std::exception {

	public:

		/**
		 * The compiler can emit many messages, only the WARING+ ones
		 * get printed to output by default, INFO and VERBOSE messages can
		 * be enabled using switches.
		 */
		enum Severity {
			VERBOSE = 0b0001,
			INFO    = 0b0010,
			WARNING = 0b0100,
			ERROR   = 0b1000,
		};

		class Node {

			private:

				friend class MessageBuilder;

				Severity m_severity = VERBOSE;
				SourceSpan m_section;
				std::string m_message;

			public:

				Node() = default;
				Node(Severity severity, SourceSpan section, std::string message) : m_severity(severity), m_section(section), m_message(message) {}

				Severity severity() const {
					return m_severity;
				}

				SourceSpan section() const {
					return m_section;
				}

				std::string message() const {
					return m_message;
				}

				std::string where() const {
					if (m_section.empty()) {
						return "";
					}

					const SourceUnit* unit = m_section.unit;
					const SourcePos pos = unit->find(m_section.begin);

					return unit->path + ":" + std::to_string(pos.line) + ":" + std::to_string(pos.column);
				}

		};

	private:

		std::string m_what = "<no details>";
		std::shared_ptr<std::list<Node>> m_nodes;

	public:

		Message() = default;
		Message(std::shared_ptr<std::list<Node>> nodes) : m_nodes(nodes) {

			// the "what" message can't be formatted and is used
			// for debugging only, as we extend std::exception this will be used as the what() message

			m_what.clear();
			m_what += "Message ";

			for (const Node& node : *nodes) {
				m_what += " [";

				Severity severity = node.severity();
				std::string location = node.where();

				switch (severity) {
					case VERBOSE: m_what += "VER "; break;
					case INFO: m_what += "INF "; break;
					case WARNING: m_what += "WRN "; break;
					case ERROR: m_what += "ERR "; break;
					default: m_what += "UNK "; break;
				}

				if (!location.empty()) {
					m_what += location;
					m_what += ": ";
					m_what += node.section().quote();
					m_what += " -> ";
				}

				m_what += node.message();
				m_what += "]";
			}
		}

		/**
		 * Get the combined severity of this set of message nodes
		 */
		Severity severity() const {
			int severity = 0;

			for (const Node& node : *m_nodes) {
				severity |= node.severity();
			}

			return (Severity) severity;
		}

		/**
		 * Get message nodes that make up this message
		 */
		const std::list<Node> nodes() const {
			return *m_nodes;
		}

		/**
		 * Get some simple message, mostly for debugging
		 */
		const char* what() const noexcept override {
			return m_what.c_str();
		}

		/**
		 * Check if this message contain an error
		 */
		bool error() const {
			for (const Node& node : *m_nodes) {
				if (node.severity() == ERROR) {
					return true;
				}
			}

			return false;
		}

		/**
		 * @see MessageBuilder
		 */
		static MessageBuilder of();

};

/**
 * All compiler output is directed into this class,
 *
 */
class MessageSink {

	public:

		using Printer = std::function<void(const Message&)>;

	private:

		bool failed = false;
		Printer m_printer = [] (const Message&) noexcept -> void {};

		MessageSink() = default;

		static MessageSink& instance();

	public:

		static void printer(const Printer& printer);
		static void feed(const Message& message);

		static void clear();
		static bool error();

		/// FIXME: remove and replace with guard
		static void disable();

};

/**
 * This class should be used to construct Message objects
 * in the compiler.
 */
class MessageBuilder {

	private:

		struct NodeSet {

			std::shared_ptr<std::list<Message::Node>> nodes;

		};

		NodeSet set;

		MessageBuilder& setMessage(const std::string& message, Message::Severity severity) {
			set.nodes->back().m_severity = severity;
			set.nodes->back().m_message = message;
			return *this;
		}

	public:

		MessageBuilder()
			: set(std::make_shared<std::list<Message::Node>>()) {
			set.nodes->emplace_back(); // start with a default, empty node
		}

		MessageBuilder(NodeSet& previous)
			: set(previous) {
			set.nodes->emplace_back(); // move to the next node
		}

		MessageBuilder& source(SourceSpan span) {
			set.nodes->back().m_section = span;
			return *this;
		}

		MessageBuilder& error(const std::string& message) {
			return setMessage(message, Message::ERROR);
		}

		MessageBuilder& warn(const std::string& message) {
			return setMessage(message, Message::WARNING);
		}

		MessageBuilder& info(const std::string& message) {
			return setMessage(message, Message::INFO);
		}

		MessageBuilder& debug(const std::string& message) {
			return setMessage(message, Message::VERBOSE);
		}

		MessageBuilder next() {

			// yes we copy here,
			// NodeSet contains a shared pointer to the node list, its ok
			return {set};
		}

		void report() const {
			MessageSink::feed(done());
		}

		[[noreturn]] void raise() const {
			Message message = done();
			MessageSink::feed(message);
			throw message;
		}

		Message done() const {
			return Message {set.nodes};
		}

};

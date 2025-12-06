#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <bits/locale_facets_nonio.h>

namespace argx {

	namespace util {

		std::pair<std::string_view, std::string_view> split(std::string_view str, char separator) {
			auto it = str.find(separator);

			if (it != std::string_view::npos) {
				return {str.substr(0, it), str.substr(it + 1, str.size())};
			}

			return {str, ""};
		}

		std::string str_from_char(char c) {
			std::string value;
			value.resize(1);
			value[0] = c;
			return value;
		}

		std::string padding(size_t current, size_t target) {
			if (current >= target) {
				return "";
			}

			size_t count = target - current;
			std::string padded;
			padded.resize(count);

			for (size_t i = 0; i < count; i ++) {
				padded[i] = ' ';
			}

			return padded;
		}

		std::string pad_to_length(const std::string& str, size_t length) {
			return str + padding(str.length(), length);
		}

	}

	enum usage {

		/**
		 * Flag expects no value, but you can check for its presence.
		 * See the string and list types if you ish to allow values for your flag.
		 */
		flag,

		/**
		 * Single value is expected after the flag, do note that is the flag
		 * is allowed to be used multiple times it can still have multiple values.
		 */
		string,

		/**
		 * Expect a list of values of unspecified length (at least one),
		 * if the flag is allowed to be used multiple times the value lists are joined into one.
		 */
		list

	};

	/*
	 * class argument
	 */

	class argument {

		private:

			friend class builder;
			friend class parsed;

			std::string_view m_category;
			std::string m_long_name;
			char m_short_name = 0;
			std::string m_description;
			bool m_reusable = false;
			usage m_type = flag;
			std::string m_fallback;
			std::vector<std::string> m_conflicts;

		public:

			argument(const std::string_view& category)
				: m_category(category) {
			}

			std::string str() const {
				return m_long_name.empty() ? "-" + util::str_from_char(m_short_name) : "--" + m_long_name;
			}

		public:

			argument& name(const std::string& long_name) {
				m_long_name = long_name;
				return *this;
			}

			argument& name(char short_name) {
				m_short_name = short_name;
				return *this;
			}

			/**
			 * An argument can be exclusive with other arguments, this method adds
			 * flags that cannot be used together with this flag.
			 */
			argument& conflicts(const std::string& name) {
				m_conflicts.push_back(name);
				return *this;
			}

			/**
			 * Description that will be used in the auto generated help page,
			 * see 'add_help()', 'get_help()'.
			 */
			argument& detail(const std::string& description) {
				m_description = description;
				return *this;
			}

			/**
			 * The default value of this flag the flag wasn't used,
			 * for this option to work use the 'string' or 'list' argument type.
			 */
			argument& fallback(const std::string_view& value) {
				m_fallback = value;
				return *this;
			}

			/**
			 * Allow this flag to be specified multiple time, the values (if the
			 * flag allows any values) are joined together.
			 */
			argument& reusable() {
				m_reusable = true;
				return *this;
			}

			/**
			 * Specify argument type, that is, what kind of data (if any)
			 * it should accept. The default is not accepting any value - being a flag.
			 */
			argument& type(usage argument_type) {
				m_type = argument_type;
				return *this;
			}

	};

	/*
	 * class option
	 */

	class option {

		private:

			friend class parsed;
			friend class builder;

			std::shared_ptr<argument> m_definition;
			int m_usages = 0;
			std::vector<std::string> m_values;

		public:

			option(std::shared_ptr<argument> definition)
				: m_definition(definition) {
			}

			operator bool() const {
				return m_usages != 0;
			}

			int count() const {
				return m_usages;
			}

			const std::vector<std::string>& values() const {
				return m_values;
			}

			std::string value(const std::string_view& separator = ";") const {
				std::string result = "";

				for (const std::string& value : m_values) {
					if (!result.empty()) {
						result += separator;
					}

					result += value;
				}

				return result;
			}

	};

	/*
	 * class parsed
	 */

	class parsed {

		private:

			friend class builder;

			size_t max_arg_length = 0;
			std::string m_invocation;

			std::vector<std::shared_ptr<option>> m_options;
			std::unordered_map<std::string_view, std::shared_ptr<option>> m_long_options;
			std::unordered_map<char, std::shared_ptr<option>> m_short_options;

			void create(const std::shared_ptr<argument>& argument) {
				auto ptr = std::make_shared<option>(argument);
				size_t length = argument->m_long_name.length();

				if (length > max_arg_length) {
					max_arg_length = length;
				}

				m_options.emplace_back(ptr);
				if (!argument->m_long_name.empty()) m_long_options[argument->m_long_name] = ptr;
				if (argument->m_short_name != 0) m_short_options[argument->m_short_name] = ptr;
			}

			option& get(const argument& argument) {
				return (argument.m_short_name != 0) ? get(argument.m_short_name) : get(argument.m_long_name);
			}

			bool conflicts(const std::string_view& name) {
				if (get(name).m_usages > 0) {
					return true;
				}

				if (name.length() == 1 && get(name[0]).m_usages > 0) {
					return true;
				}

				return false;
			}

		public:

			void dump() const {
				printf("Listing state of all defined options:\n");

				for (auto& ptr : m_options) {
					std::string name = ptr->m_definition->str();
					std::string padded = util::pad_to_length(name, max_arg_length);

					printf(" %s  %s  ", padded.c_str(), ptr->m_usages == 0 ? "false" : "true");
					size_t count = ptr->m_values.size();

					for (size_t i = 0; i < count; i ++) {
						printf("'%s'", ptr->m_values.at(i).c_str());

						if (i != count - 1) {
							printf(", ");
						}
					}

					printf("\n");
				}
			}

			option& get(char name) {
				auto it = m_short_options.find(name);

				if (it == m_short_options.end()) {
					throw std::runtime_error {"Option '-" + util::str_from_char(name) + "' undefined"};
				}

				return *it->second;
			}

			option& get(const std::string_view& name) {
				auto it = m_long_options.find(name);

				if (it == m_long_options.end()) {
					throw std::runtime_error {"Option '--" + std::string(name) + "' undefined"};
				}

				return *it->second;
			}

			/**
			 * Returns the path or name that was used to invoke the program
			 */
			const std::string& invocation() const {
				return m_invocation;
			}

	};

	/*
	 * class builder
	 */

	class builder {

		public:

			static int default_error_handler(const std::string& message) {
				printf("Syntax error! %s\n", message.c_str());
				printf("Try running with '--help' for more information.\n");

				return 1;
			}

		private:

			bool m_auto_help = false;
			std::string_view m_description;
			std::string_view m_category = "OPTIONS";

			std::function<int(const std::string& message)> error_handler = default_error_handler;
			std::vector<std::shared_ptr<argument>> args;

			[[noreturn]] void error(const std::string& message) {
				std::exit(error_handler(message));
			}

			argument& find(std::string_view name) {
				for (std::shared_ptr<argument>& arg : args) {
					if (arg->m_long_name == name) return *arg;
				}

				error("Unknown flag '--" + std::string(name) + "' used.");
			}

			argument& find(char name) {
				for (std::shared_ptr<argument>& arg : args) {
					if (arg->m_short_name == name) return *arg;
				}

				error("Unknown flag '-" + util::str_from_char(name) + "' used.");
			}

			option& get_option(parsed& parsed, argument& arg) {
				try {
					option& option = parsed.get(arg);
					option.m_usages ++;

					if (!arg.m_reusable && option.m_usages != 1) {
						error("Flag '" + arg.str() + "' can be used at most once.");
					}

					return option;
				} catch (const std::exception& e) {
					error("Unknown flag '" + arg.str() + "' used.");
				}
			}

			void verify(parsed& parsed) {

				for (const auto& ptr : parsed.m_options) {
					if (ptr->m_usages) {

						for (const std::string& excluded : ptr->m_definition->m_conflicts) {
							if (parsed.conflicts(excluded)) {
								std::string other = excluded.length() == 1 ? "-" + util::str_from_char(excluded[0]) : "--" + std::string(excluded);
								error("Flag '" + ptr->m_definition->str() + "' can't be used with '" + other + "'.");
							}
						}

					} else if (!ptr->m_definition->m_fallback.empty()) {
						ptr->m_values.push_back(ptr->m_definition->m_fallback);
					}
				}

			}

		public:

			builder(const std::string_view& description = "Untitled terminal application.\n")
				: m_description(description) {
			}

			void category(const std::string_view& name) {
				m_category = name;
			}

			argument& add() {
				return *args.emplace_back(std::make_shared<argument>(m_category));
			}

			argument& add(const std::string& long_name, char short_name = 0) {
				return add().name(long_name).name(short_name);
			}

			argument& add(char short_name, const std::string& long_name = "") {
				return add().name(long_name).name(short_name);
			}

			void add_help() {
				if (m_auto_help == false) {
					add("help", 'h').reusable().detail("Show this help page and exit.");
					m_auto_help = true;
				}
			}

			std::string get_help(int line_indent = 1, int detail_indent = 2, bool show_defaults = true) const {
				std::string result;

				result += m_description;

				std::string line_prefix = util::padding(0, line_indent);
				std::string detail_prefix = util::padding(0, detail_indent);

				size_t max_length = 0;

				for (const std::shared_ptr<argument>& arg : args) {
					size_t length = arg->m_long_name.length();

					if (length > max_length) {
						max_length = length;
					}
				}

				std::string_view current_category;

				for (const std::shared_ptr<argument>& arg : args) {

					if (arg->m_category != current_category) {
						current_category = arg->m_category;
						result += "\n";
						result += current_category;
						result += "\n";
					}

					result += line_prefix;
					result += (arg->m_short_name == 0) ? "  " : "-" + util::str_from_char(arg->m_short_name);
					result += arg->m_long_name.empty() ? "  " : ", --" + arg->m_long_name;

					result += util::padding(arg->m_long_name.length(), max_length);
					result += detail_prefix;
					result += arg->m_description;

					if (show_defaults && !arg->m_fallback.empty()) {
						result += " (default: '";
						result += arg->m_fallback;
						result += "')";
					}

					result += "\n";
				}

				return result;
			}

			parsed parse(const std::vector<std::string>& args) {

				// normally we should always get at last one argument so this means something is wrong
				if (args.empty()) {
					error("No invocation path in argument list, was the syscall invalid?");
				}

				bool expect_flag = true;
				option* collector = nullptr;

				parsed result;
				result.m_invocation = args[0];

				for (const std::shared_ptr<argument>& arg : this->args) {
					result.create(arg);
				}

				for (size_t i = 1; i < args.size(); i ++) {
					const std::string_view arg = args[i];

					if (expect_flag) {

						// bail out if this token doesn't look like a flag and we have an active collector set
						if (!arg.starts_with("-") && collector) {
							goto try_collect;
						}

						collector = nullptr;
						auto [flag, value] = util::split(arg, '=');

						if (arg.ends_with('=') && value.empty()) {
							error("Equal sign (=) must be followed by value when used.");
						}

						if (flag.starts_with("--") && flag.size() > 2) {
							argument& argument = find(flag.substr(2));
							option& option = get_option(result, argument);

							if (argument.m_type == string) {
								if (!value.empty()) {
									option.m_values.emplace_back(value);
									continue;
								}

								expect_flag = false;
								collector = &option;
								continue;
							}

							if (argument.m_type == list) {
								if (!value.empty()) {

									// '=' implicates only one value
									option.m_values.emplace_back(value);
									continue;
								}

								// we need at least one value
								expect_flag = false;
								collector = &option;
								continue;
							}

							continue;
						}

						if (flag.starts_with("-") && flag.size() > 1) {
							size_t i = 0;

							for (char c : arg.substr(1)) {
								i ++;

								argument& argument = find(c);
								option& option = get_option(result, argument);

								if (argument.m_type == usage::flag) {
									continue;
								}

								// if this short argument accepts values it MUST be the last - everything after it
								// will be considered the value, if it is the last char then the value after the flag will be used
								// but otherwise single 'inline' value will be read, even for lists.
								if (i != arg.size() - 1) {
									std::string_view inline_value = arg.substr(i + 1, arg.size());
									option.m_values.emplace_back(inline_value);
									break;
								}

								expect_flag = false;
								collector = &option;
								break;
							}

							continue;
						}

						error("Expected flag, but got '" + std::string(arg) + "'.");
					}

					try_collect:

					if (collector) {
						expect_flag = true;
						collector->m_values.emplace_back(arg);
						continue;
					}

					error("Unable to parse '" + std::string(arg) + "'.");
				}

				verify(result);

				if (m_auto_help) {
					if (result.get("help")) {
						printf("%s", get_help().c_str());
						exit(0);
					}
				}

				return result;
			}

			parsed parse(int argc, char* argv[]) {
				std::vector<std::string> args;
				args.reserve(argc);

				for (int i = 0; i < argc; i++) {
					args.emplace_back(argv[i]);
				}

				return parse(args);
			}

	};

}

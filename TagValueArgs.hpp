#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>

struct TagValueArg {
	enum RequiredOrOptional {
		Required,
		Optional
	};

	TagValueArg(
		std::string const &tag_,
		std::function< bool(std::string const &, std::string *) > const &parse_,
		std::function< bool(std::string *) > const &check_,
		std::string const &help_,
		RequiredOrOptional required_)
			: tag(tag_), parse(parse_), check(check_), help(help_), required(required_) {
	}
	static bool dont_check(std::string *err) {
		assert(err);
		return false;
	}
	template< typename T >
	static TagValueArg simple(std::string const &tag_, T *value, std::string const &help_, RequiredOrOptional req_ = Optional) {
		return TagValueArg(
			tag_,
			[value](std::string const &arg, std::string *err) -> bool {
				assert(err);
				std::istringstream str(arg);
				if (!(str >> *value)) {
					*err = "Failed to read " + std::string(typeid(T).name()) + " from '" + arg + "'.";
					return false;
				}
				char extra;
				if (str >> extra) {
					*err = "Found trailing junk in '" + arg + "'.";
					return false;
				}
				return true;
			},
			dont_check,
			help_,
			req_);
	}
	static TagValueArg simple(std::string const &tag_, std::string *value, std::string const &help_, RequiredOrOptional req_ = Optional) {
		return TagValueArg(
			tag_,
			[value](std::string const &arg, std::string *err) -> bool {
				assert(err);
				*value = arg;
				return true;
			},
			dont_check,
			help_,
			req_);
	}

	std::string tag;
	std::function< bool(std::string const &, std::string *) > parse;
	std::function< bool(std::string *) > check;
	std::string help;
	RequiredOrOptional required;
};


struct TagValueArgs : std::vector< TagValueArg > {
	template< typename ITER >
	bool parse(ITER begin, ITER end, std::string *errs) {
		assert(errs);
		*errs = "";
		bool have_errs = false;
		std::vector< bool > parsed(this->size(), false);

		for (; begin != end; ++begin) {
			std::string tag = *begin;
			std::string value = "";
			{
				auto idx = tag.find(':');
				if (idx != std::string::npos) {
					value = tag.substr(idx+1);
					tag = tag.substr(0,idx);
				}
			}
			bool found = false;
			for (auto const &arg : *this) {
				if (arg.tag == tag) {
					parsed[&arg - this->data()] = true;
					found = true;
					std::string err;
					if (!arg.parse(value, &err)) {
						if (!have_errs) {
							have_errs = true;
						} else {
							*errs += '\n';
						}
						*errs += tag + " Parse Error:" + err;
					}
					continue;
				}
			}
			if (!found) {
				if (!have_errs) {
					have_errs = true;
				} else {
					*errs += '\n';
				}
				*errs += "Invalid Tag: '" + tag + "'";
			}
		}
		for (uint32_t i = 0; i < this->size(); ++i) {
			if (!parsed[i] && (*this)[i].required == TagValueArg::Required) {
				if (!have_errs) {
					have_errs = true;
				} else {
					*errs += '\n';
				}
				*errs += "Missing Required Argument: '" + (*this)[i].tag + "'";
			}
		}
		return !have_errs;
	}
	std::string usage(std::string const &command) const {
		std::string u = "Usage:\n\t" + command;
		for (auto const &arg : *this) {
			if (arg.required == TagValueArg::Required) {
				u += " <" + arg.tag + ":...>";
			} else {
				u += " [" + arg.tag + ":...]";
			}
		}
		for (auto const &arg : *this) {
			u += '\n';
			u += "\t" + arg.tag + ": " + arg.help;
		}
		return u;
	}
};

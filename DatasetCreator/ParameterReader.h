#pragma once
#include <string>
#include <map>

template<typename ValueType>
class ParameterReader {
public:
	ParameterReader(const std::string& config_file) :config_file_(config_file) {};
	ParameterReader() = default;

	std::map<std::string, ValueType> getMap() const {
		return this->parameters_;
	};

	void registerParam(const std::string& key) {

		std::ifstream c_file(this->config_file_);
		if (c_file.is_open()) {
			std::string line;
			while (std::getline(c_file, line))
			{
				// remove spaces from actual line
				line.erase(std::remove_if(line.begin(), line.end(), isspace),
					line.end());
				// parse comment or empty line
				if (line[0] == '#' || line.empty()) {
					continue;
				}
				const auto del_pos = line.find(":");
				const auto act_key = line.substr(0, del_pos);
				if (act_key.compare(key) == 0) {
					// convert key to templated type
					parameters_[key] = convertKey<ValueType>(line.substr(del_pos + 1));
					return;
				}
			}
		}
		else {
			throw std::invalid_argument("The speficied Parameterfile " + config_file_ + " does not exist!");
		}

		// This line should never be reached
		throw std::invalid_argument("The Parameter " + key + " does not exist! Check Parameterfile!");
	};

	ValueType getParam(const std::string& key) {
		if (this->parameters_.find(key) == this->parameters_.end()) {
			throw std::invalid_argument("Key " + key + "is not registered! Check its name and value type!");
		}
		return parameters_[key];
	};

private:
	std::map<std::string, ValueType> parameters_;
	const std::string config_file_;

	template<typename ValueType>
	ValueType convertKey(const std::string& key) const {
		std::stringstream converter(key);
		ValueType value;
		converter >> value;
		return value;
	};

	// explicit specialization is needed for strings
	template<>
	std::string convertKey<std::string>(const std::string& key) const {
		const std::string value = key;
		return value;
	}

	// needed for bools
	template<>
	bool convertKey<bool>(const std::string& key) const {
		std::stringstream converter(key);
		bool value;
		converter >> std::boolalpha >> value;
		return value;
	}




};


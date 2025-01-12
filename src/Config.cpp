// This file is part of ElasticTabstops.
// 
// Copyright (C)2016 Justin Dailey <dail8859@yahoo.com>
// 
// ElasticTabstops is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdio.h>
#include <sstream>
#include "Config.h"

template <typename T, typename U>
static std::string join(const std::vector<T> &v, const U &delim) {
	std::stringstream ss;
	for (size_t i = 0; i < v.size(); ++i) {
		if (i != 0) ss << delim;
		ss << v[i];
	}
	return ss.str();
}

static std::vector<std::string> split(std::string const &str, const char delim) {
	size_t start;
	size_t end = 0;
	std::vector<std::string> out;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}

	return out;
}

const wchar_t *GetIniFilePath(const NppData *nppData) {
	static wchar_t iniPath[MAX_PATH];
	SendMessage(nppData->_nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniPath);
	wcscat_s(iniPath, MAX_PATH, L"\\StretchyTabs.ini");
	return iniPath;
}

void ConfigLoad(const NppData *nppData, Configuration *config) {
	const wchar_t *iniPath = GetIniFilePath(nppData);

	FILE *file = _wfopen(iniPath, L"r");

	if (file == nullptr) return;

	char line[256];
	while (true) {
		if (fgets(line, 256, file) == NULL) break;

		// Ignore comments and blank lines
		if (line[0] == ';' || line[0] == '\r' || line[0] == '\n') continue;

		// TODO: if this next section gets too bloated/complicated come up with a better way

		if (strncmp(line, "enabled ", 8) == 0) {
			char *c = &line[8];
			while (isspace(*c)) c++;
			config->enabled = strncmp(c, "true", 4) == 0;
		}
		else if (strncmp(line, "extensions ", 11) == 0) {
			if (!config->file_extensions.empty()) {
				config->file_extensions.clear();
			}

			// Strip the newline
			line[strcspn(line, "\r\n")] = 0;

			config->file_extensions = split(&line[11], ' ');
		}
		else if (strncmp(line, "padding ", 8) == 0) {
			char *c = &line[8];
			while (isspace(*c)) c++;

			config->min_padding = strtol(c, nullptr, 10);

			// The above could fail or the user types something crazy
			if (config->min_padding > 256) config->min_padding = 256;
			if (config->min_padding == 0) config->min_padding = 2;
		}
		else if (strncmp(line, "match_all_tabs_in_block ", 24) == 0) {
			char* c = line + 24;
			while (isspace(*c)) c++;
			config->match_all_tabs_in_block = strncmp(c, "true", 4) == 0;
		} 
		else if (strncmp(line, "convert_leading_tabs_to_spaces ", 31) == 0) {
			char *c = &line[31];
			while (isspace(*c)) c++;
			config->convert_leading_tabs_to_spaces = strncmp(c, "true", 4) == 0;
		}
	}

	fclose(file);
}

void ConfigSave(const NppData *nppData, const Configuration *config) {
	const wchar_t *iniPath = GetIniFilePath(nppData);

	FILE *file = _wfopen(iniPath, L"w");
	if (file == nullptr) return;

	fputs("; Configuration for StretchyTabs.\n; Saving this file will immediately apply the settings.\n\n", file);

	// Whether or not it is enabled
	fputs("; Whether stretchy tabs are enabled or not: true or false\n", file);
	fprintf(file, "enabled %s\n\n", config->enabled == true ? "true" : "false");

	// The file extensions to appy it to
	fputs("; File extensions to apply stretchy tabs. For example...\n", file);
	fputs(";   \"extensions *\" will apply it to all files\n", file);
	fputs(";   \"extensions .c .h .cpp .hpp\" will apply it to only C/C++ files\n", file);
	fputs(";   \"extensions !.txt !.py *\" will apply it to all files except for text and Python files\n", file);
	if (!config->file_extensions.empty())
		fprintf(file, "extensions %s\n\n", join(config->file_extensions, ' ').c_str());
	else
		fprintf(file, "extensions *\n\n");

	// Minimum padding
	fputs("; Minimum padding in characters. Must be > 0\n", file);
	fprintf(file, "padding %Iu\n\n", config->min_padding);

	// Match all tabs in block
	fputs("; Match all tabs in blocks with variable number of tabs: true or false\n", file);
	fprintf(file, "match_all_tabs_in_block %s\n\n", config->match_all_tabs_in_block == true ? "true" : "false");

	// Leading tabs
	fputs("; Convert leading tabs to spaces: true or false\n", file);
	fprintf(file, "convert_leading_tabs_to_spaces %s\n", config->convert_leading_tabs_to_spaces == true ? "true" : "false");

	fclose(file);
}

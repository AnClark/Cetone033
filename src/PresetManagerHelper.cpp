#include "PresetManager.h"
#include "defines.h" // For constants like DEFAULT_USER_BANK_FILENAME

#ifdef DISTRHO_OS_WINDOWS
#include <shlobj.h>
#include <windows.h>

#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif

// ============================================================================
// File I/O Helpers
// ============================================================================

String
CPresetManager::_getDefaultBankPath() const
{
	String dir = _getUserPresetsDirectory();
	if (dir.isEmpty())
		return String();

#ifdef DISTRHO_OS_WINDOWS
	return dir + "\\" + DEFAULT_USER_BANK_FILENAME;
#else
	return dir + "/" + DEFAULT_USER_BANK_FILENAME;
#endif
}

bool
CPresetManager::_fileExists(const String& path) const
{
	if (path.isEmpty())
		return false;

	FILE* f = std::fopen(path.buffer(), "rb");
	if (f) {
		std::fclose(f);
		return true;
	}
	return false;
}

String
CPresetManager::_readFileContent(const String& path) const
{
	FILE* f = std::fopen(path.buffer(), "rb");
	if (!f) {
		d_stderr("Failed to open file for reading: %s", path.buffer());
		return String();
	}

	// Get file size
	std::fseek(f, 0, SEEK_END);
	long size = std::ftell(f);
	std::fseek(f, 0, SEEK_SET);

	if (size <= 0) {
		std::fclose(f);
		return String();
	}

	// Read content
	char* buffer = (char*)std::malloc(size + 1);
	if (!buffer) {
		std::fclose(f);
		return String();
	}

	size_t read = std::fread(buffer, 1, size, f);
	buffer[read] = '\0';
	std::fclose(f);

	String content(buffer);
	std::free(buffer);

	return content;
}

bool
CPresetManager::_writeFileContent(const String& path,
								  const String& content) const
{
	FILE* f = std::fopen(path.buffer(), "wb");
	if (!f) {
		d_stderr("Failed to open file for writing: %s", path.buffer());
		return false;
	}

	size_t written = std::fwrite(content.buffer(), 1, content.length(), f);
	std::fclose(f);

	if (written != content.length()) {
		d_stderr("Failed to write complete content to file: %s", path.buffer());
		return false;
	}

	return true;
}

bool
CPresetManager::_createDirectoryIfNeeded(const String& dirPath) const
{
	if (dirPath.isEmpty())
		return false;

#ifdef DISTRHO_OS_WINDOWS
	// Check if directory exists
	DWORD attrib = GetFileAttributesA(dirPath.buffer());
	if (attrib != INVALID_FILE_ATTRIBUTES &&
		(attrib & FILE_ATTRIBUTE_DIRECTORY))
		return true; // Directory already exists

	// Create directory
	if (CreateDirectoryA(dirPath.buffer(), NULL) ||
		GetLastError() == ERROR_ALREADY_EXISTS)
		return true;

	d_stderr("Failed to create directory: %s", dirPath.buffer());
	return false;
#else
	// Check if directory exists
	struct stat st;
	if (stat(dirPath.buffer(), &st) == 0 && S_ISDIR(st.st_mode))
		return true; // Directory already exists

	// Create directory
	if (mkdir(dirPath.buffer(), 0755) == 0 || errno == EEXIST)
		return true;

	d_stderr("Failed to create directory: %s", dirPath.buffer());
	return false;
#endif
}

// ============================================================================
// Bank Management Helpers
// ============================================================================

String
CPresetManager::_getBanksDirectory() const
{
	String baseDir = _getUserPresetsDirectory();
	if (baseDir.isEmpty())
		return String();

#ifdef DISTRHO_OS_WINDOWS
	return baseDir + "\\" USER_PRESET_BANK_SUBDIR;
#else
	return baseDir + "/" USER_PRESET_BANK_SUBDIR;
#endif
}

bool
CPresetManager::_isDefaultUserBank(const char* bankName) const
{
	return std::strcmp(bankName, DEFAULT_USER_BANK_NAME) == 0;
}

bool
CPresetManager::_isFactoryBank(const char* bankName) const
{
	return std::strcmp(bankName, FACTORY_BANK_NAME) == 0;
}

String
CPresetManager::_getBankFilePath(const char* bankName)
{
	if (_isDefaultUserBank(bankName)) {
		// Default User Bank uses the special UserPresets.c033bank file
		return _getDefaultBankPath();
	}

	// Other banks are in the Banks directory
	String banksDir = _getBanksDirectory();
	if (banksDir.isEmpty())
		return String();

#ifdef DISTRHO_OS_WINDOWS
	return banksDir + "\\" + String(bankName) + USER_PRESET_BANK_EXTENSION;
#else
	return banksDir + "/" + String(bankName) + USER_PRESET_BANK_EXTENSION;
#endif
}

bool
CPresetManager::_loadBankFromFile(const String& filePath, PresetBank& outBank)
{
	if (!_fileExists(filePath))
		return false;

	String jsonContent = _readFileContent(filePath);
	if (jsonContent.isEmpty())
		return false;

	return deserializeBankFromJSON(jsonContent, outBank);
}

bool
CPresetManager::_saveBankToFile(const String& filePath, const PresetBank& bank)
{
	String jsonContent = serializeBankToJSON(bank);
	if (jsonContent.isEmpty())
		return false;

	return _writeFileContent(filePath, jsonContent);
}

void
CPresetManager::_sanitizeBankName(String& bankName) const
{
	// 1. Replace illegal filesystem characters with underscore
	bankName.replace('/', '_');
	bankName.replace('\\', '_');
	bankName.replace(':', '_');
	bankName.replace('*', '_');
	bankName.replace('?', '_');
	bankName.replace('\"', '_');
	bankName.replace('<', '_');
	bankName.replace('>', '_');
	bankName.replace('|', '_');
	
	// 2. Trim leading spaces
	while (bankName.length() > 0 && bankName[0] == ' ') {
		// Create a temporary copy to avoid dangling pointer
		String temp(bankName.buffer() + 1);
		bankName = temp;
	}
	
	// 3. Trim trailing spaces and dots (Windows restriction)
	while (bankName.length() > 0) {
		const char lastChar = bankName[bankName.length() - 1];
		if (lastChar == ' ' || lastChar == '.') {
			bankName.truncate(bankName.length() - 1);
		} else {
			break;
		}
	}
	
	// 4. Check for Windows reserved names (case-insensitive)
	// Reserved: CON, PRN, AUX, NUL, COM1-9, LPT1-9
	if (bankName.length() > 0) {
		String upperName = bankName.asUpper();
		const char* reserved[] = {
			"CON", "PRN", "AUX", "NUL",
			"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
			"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
		};
		
		for (int i = 0; i < 22; i++) {
			if (upperName == reserved[i]) {
				// Prefix with underscore to avoid reserved name
				bankName = String("_") + bankName;
				break;
			}
		}
	}
	
	// 5. Ensure the result is not empty (fallback to "Unnamed" if all chars were illegal)
	if (bankName.length() == 0) {
		bankName = String("Unnamed");
	}
}

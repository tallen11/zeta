#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


const uint8_t cContentsSize = 6;
const char *const cContents[] = {
	"#include <stdio.h>\n",
	"\n",
	"int main(int argc, char const *argv[])\n",
	"{\n",
	"\treturn 0;\n",
	"}\n"
};

const uint8_t makeFileContentsSize = 11;
const char *const makeFileContents[] = {
	"CC=clang\n",
	"DEBUG=-g\n",
	"CFLAGS=-Wall -Wextra -pedantic -std=c11 $(DEBUG)\n",
	"\n",
	"all: {F}\n",
	"\t$(CC) {F} -o {E} $(CFLAGS)\n",
	"\n",
	"clean:\n",
	"\trm -rf *.o\n",
	"\trm -rf {E}\n",
	"\trm -rf *.dSYM"
};

void printUsage();
char* replaceAll(const char *pattern, const char *repl, const char *str);

int main(int argc, char const *argv[])
{
	if (argc != 3) {
		printUsage();
		return 1;
	}

	if (strcmp(argv[1], "gen") != 0) {
		printUsage();
		return 1;
	}

	if (strlen(argv[2]) == 0) {
		printUsage();
		return 1;
	}

	const char *cFileNameRoot = argv[2];
	const char *ext = ".c";
	char *cFileName = calloc(strlen(cFileNameRoot) + strlen(ext) + 1, sizeof(char));
	strcat(cFileName, cFileNameRoot);
	strcat(cFileName, ext);

	const char *makeFileName = "Makefile";

	char buff[1024];
	if (getcwd(buff, sizeof(buff)) == NULL) {
		printf("Couldn't get current working directory!\n");
		return 1;
	}

	char *cFilePath = calloc(strlen(buff) + strlen(cFileName) + 2, sizeof(char));
	strcat(cFilePath, buff);
	strcat(cFilePath, "/");
	strcat(cFilePath, cFileName);

	if (access(cFilePath, F_OK) == -1) {
		FILE *f = fopen(cFilePath, "w");
		for (uint8_t i = 0; i < cContentsSize; ++i) {
			fputs(cContents[i], f);
		}

		fclose(f);
		printf("Generated %s\n", cFileName);
	}

	char *makeFilePath = calloc(strlen(buff) + strlen(makeFileName) + 2, sizeof(char));
	strcat(makeFilePath, buff);
	strcat(makeFilePath, "/");
	strcat(makeFilePath, makeFileName);

	if (access(makeFilePath, F_OK) == -1) {
		FILE *f = fopen(makeFilePath, "w");
		for (uint8_t i = 0; i < makeFileContentsSize; ++i) {
			char *pass1 = replaceAll("{F}", cFileName, makeFileContents[i]);
			char *pass2 = replaceAll("{E}", cFileNameRoot, pass1);
			fputs(pass2, f);
		}

		fclose(f);
		printf("Generated %s\n", makeFileName);
	}

	free(makeFilePath);
	free(cFilePath);
	free(cFileName);

	return 0;
}

void printUsage()
{
	printf("Usage:\n");
	printf("\tzet gen [FILENAME]\n");
}

/* Only replaces first 10 appearances lol this is a horrible
   way of doing this. Will fix later. */
char* replaceAll(const char *pattern, const char *repl, const char *str)
{
	uint_fast32_t replacementCount = 0;
	uint_fast32_t indices[10];
	uint_fast32_t patterLength = strlen(pattern);
	uint_fast32_t replLength = strlen(repl);
	uint_fast32_t strLength = strlen(str);

	if (strLength == 0 || patterLength == 0 || replLength == 0) {
		// TODO: Come fix this
		return "";
	}

	for (uint_fast32_t i = 0; i < strLength; ++i) {
		char c = str[i];
		char currentP = pattern[0];
		if (c == currentP) {
			for (uint_fast32_t j = 0; j < patterLength; ++j) {
				uint_fast32_t globalIndex = i + j;
				if (str[globalIndex] != pattern[j]) {
					break;
				}

				if (j == patterLength - 1) {
					indices[replacementCount] = i;
					replacementCount++;
					break;
				}
			}
		}

		if (replacementCount == 9) {
			break;
		}
	}

	if (replacementCount == 0) {
		return (char *)str;
	}

	uint_fast32_t newStringLength = strLength + 
									(replacementCount * replLength) - 
									(replacementCount * patterLength);
	uint_fast32_t currentReplacementIndex = 0;
	uint_fast32_t currentStringIndex = 0;
	uint_fast32_t oldStringIndex = 0;
	char *newString = calloc(newStringLength + 2, sizeof(char));
	while (currentStringIndex < newStringLength) {
		if (oldStringIndex == indices[currentReplacementIndex]) {
			for (uint_fast32_t j = 0; j < replLength; j++) {
				uint_fast32_t globalIndex = currentStringIndex + j;
				newString[globalIndex] = repl[j];
			}

			currentReplacementIndex++;
			currentStringIndex += replLength;
			oldStringIndex += patterLength;
		} else {
			newString[currentStringIndex] = str[oldStringIndex];
			currentStringIndex++;
			oldStringIndex++;
		}
	}

	newString[newStringLength] = '\0';

	return newString;
}
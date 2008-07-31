#include <inttypes.h>
#include <stdio.h>

#include <parserutils/input/inputstream.h>

#include <hubbub/hubbub.h>

#include "utils/utils.h"

#include "tokeniser/tokeniser.h"

#include "testutils.h"

static void token_handler(const hubbub_token *token, void *pw);

static void *myrealloc(void *ptr, size_t len, void *pw)
{
	UNUSED(pw);

	return realloc(ptr, len);
}

int main(int argc, char **argv)
{
	parserutils_inputstream *stream;
	hubbub_tokeniser *tok;
	hubbub_tokeniser_optparams params;
	FILE *fp;
	size_t len, origlen;
#define CHUNK_SIZE (4096)
	uint8_t buf[CHUNK_SIZE];

	if (argc != 3) {
		printf("Usage: %s <aliases_file> <filename>\n", argv[0]);
		return 1;
	}

	/* Initialise library */
	assert(hubbub_initialise(argv[1], myrealloc, NULL) == HUBBUB_OK);

	stream = parserutils_inputstream_create("UTF-8", 0, NULL,
			myrealloc, NULL);
	assert(stream != NULL);

	tok = hubbub_tokeniser_create(stream, myrealloc, NULL);
	assert(tok != NULL);

	params.token_handler.handler = token_handler;
	params.token_handler.pw = NULL;
	assert(hubbub_tokeniser_setopt(tok, HUBBUB_TOKENISER_TOKEN_HANDLER,
			&params) == HUBBUB_OK);

	fp = fopen(argv[2], "rb");
	if (fp == NULL) {
		printf("Failed opening %s\n", argv[2]);
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	origlen = len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	while (len >= CHUNK_SIZE) {
		fread(buf, 1, CHUNK_SIZE, fp);

		assert(parserutils_inputstream_append(stream,
				buf, CHUNK_SIZE) == HUBBUB_OK);

		len -= CHUNK_SIZE;

		assert(hubbub_tokeniser_run(tok) == HUBBUB_OK);
	}

	if (len > 0) {
		fread(buf, 1, len, fp);

		assert(parserutils_inputstream_append(stream,
				buf, len) == HUBBUB_OK);

		len = 0;

		assert(parserutils_inputstream_append(stream, NULL, 0) ==
				HUBBUB_OK);

		assert(hubbub_tokeniser_run(tok) == HUBBUB_OK);
	}

	fclose(fp);

	hubbub_tokeniser_destroy(tok);

	parserutils_inputstream_destroy(stream);

	assert(hubbub_finalise(myrealloc, NULL) == HUBBUB_OK);

	printf("PASS\n");

	return 0;
}

void token_handler(const hubbub_token *token, void *pw)
{
	static const char *token_names[] = {
		"DOCTYPE", "START TAG", "END TAG",
		"COMMENT", "CHARACTERS", "EOF"
	};
	size_t i;

	UNUSED(pw);

	printf("%s: ", token_names[token->type]);

	switch (token->type) {
	case HUBBUB_TOKEN_DOCTYPE:
		printf("'%.*s' %sids:\n",
				(int) token->data.doctype.name.len,
				token->data.doctype.name.ptr,
				token->data.doctype.force_quirks ?
						"(force-quirks) " : "");

		if (token->data.doctype.public_missing)
			printf("\tpublic: missing\n");
		else
			printf("\tpublic: '%.*s'\n",
				(int) token->data.doctype.public_id.len,
				token->data.doctype.public_id.ptr);

		if (token->data.doctype.system_missing)
			printf("\tsystem: missing\n");
		else
			printf("\tsystem: '%.*s'\n",
				(int) token->data.doctype.system_id.len,
				token->data.doctype.system_id.ptr);

		break;
	case HUBBUB_TOKEN_START_TAG:
		printf("'%.*s' %s%s\n",
				(int) token->data.tag.name.len,
				token->data.tag.name.ptr,
				(token->data.tag.self_closing) ?
						"(self-closing) " : "",
				(token->data.tag.n_attributes > 0) ?
						"attributes:" : "");
		for (i = 0; i < token->data.tag.n_attributes; i++) {
			printf("\t'%.*s' = '%.*s'\n",
					(int) token->data.tag.attributes[i].name.len,
					token->data.tag.attributes[i].name.ptr,
					(int) token->data.tag.attributes[i].value.len,
					token->data.tag.attributes[i].value.ptr);
		}
		break;
	case HUBBUB_TOKEN_END_TAG:
		printf("'%.*s' %s%s\n",
				(int) token->data.tag.name.len,
				token->data.tag.name.ptr,
				(token->data.tag.self_closing) ?
						"(self-closing) " : "",
				(token->data.tag.n_attributes > 0) ?
						"attributes:" : "");
		for (i = 0; i < token->data.tag.n_attributes; i++) {
			printf("\t'%.*s' = '%.*s'\n",
					(int) token->data.tag.attributes[i].name.len,
					token->data.tag.attributes[i].name.ptr,
					(int) token->data.tag.attributes[i].value.len,
					token->data.tag.attributes[i].value.ptr);
		}
		break;
	case HUBBUB_TOKEN_COMMENT:
		printf("'%.*s'\n", (int) token->data.comment.len,
				token->data.comment.ptr);
		break;
	case HUBBUB_TOKEN_CHARACTER:
		printf("'%.*s'\n", (int) token->data.character.len,
				token->data.character.ptr);
		break;
	case HUBBUB_TOKEN_EOF:
		printf("\n");
		break;
	}
}

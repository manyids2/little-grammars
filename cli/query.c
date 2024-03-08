// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

char *get_substring(char *line, int start, int end) {
  int len = end - start;
  char *s = malloc(sizeof(char) * (len + 1));
  for (int i = 0; i < len; i++) {
    s[i] = line[start + i];
  }
  s[len] = '\0';
  return s;
}

char *get_content(const char *fileName) {
  FILE *fp;
  char *content;
  long size = 0;

  // Read file to get size
  fp = fopen(fileName, "rb");
  if (fp == NULL) {
    return "";
  }
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp) + 1;
  fclose(fp);

  // Read file for content
  fp = fopen(fileName, "r");
  content = (char *)memset(malloc(size), '\0', size);
  fread(content, 1, size - 1, fp);
  fclose(fp);

  return content;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("USAGE: query QUERY_PATH FILE_PATH\n");
    return EXIT_FAILURE;
  }

  // Read query and file
  char *query_path    = argv[1];
  char *content_query = get_content(query_path);

  char *file_path     = argv[2];
  char *content_file  = get_content(file_path) ;
  printf("%s\n", file_path);

  // Keep language constant as `go` for now
  TSLanguage   *tree_sitter_go();
  TSParser     *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_go());

  // Parse file
  TSTree *tree = ts_parser_parse_string(parser, NULL, content_file, strlen(content_file));
  TSNode root  = ts_tree_root_node(tree);

  // Parse query
  TSQuery      *q;
  uint32_t     eo;
  TSQueryError et;
  q = ts_query_new(tree_sitter_go(), content_query, strlen(content_query), &eo, &et);
  if (et != TSQueryErrorNone) {
    printf("Error in `ts_query_new`: %lu\n", (uint64_t)et);
    return EXIT_FAILURE;
  }

  // Total number of query patterns, unique capture names and strings
  int n_patterns = ts_query_pattern_count(q);
  int n_captures = ts_query_capture_count(q);
  int n_strings  = ts_query_string_count (q);

  // Execute query on code
  TSQueryMatch  m;
  TSQueryCursor *c = ts_query_cursor_new();
  ts_query_cursor_exec(c, q, root);
  while (ts_query_cursor_next_match(c, &m)) {
    printf("  pattern: %d\n", m.pattern_index);
    for (int ci = 0; ci < m.capture_count; ci++) {
      TSQueryCapture cp    = m.captures[ci];
      TSNode         n     = cp.node;
      TSPoint        start = ts_node_start_point(n);
      TSPoint        end   = ts_node_end_point(n);
      TSTreeCursor   tt    = ts_tree_cursor_new(n);

      uint32_t length;
      // TODO: Not sure how to free, get discards qualifiers
      const char *capture_name = ts_query_capture_name_for_id(q, cp.index, &length);
      if (start.row == end.row) {
        uint32_t sb = ts_node_start_byte(n);
        uint32_t eb = ts_node_end_byte(n);
        char *text = get_substring(content_file, sb, eb);
        printf("    capture: %d - %s, start: (%d, %d), end: (%d, %d), text: " "`%s`\n",
               cp.index, capture_name, start.row, start.column, end.row, end.column, text);
        free(text);
      } else {
        printf("    capture: %s, start: (%d, %d), end: (%d, %d)\n",
               capture_name, start.row, start.column, end.row, end.column);
      }
    }
  }

  return EXIT_SUCCESS;
}

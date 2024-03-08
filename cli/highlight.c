// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

const char *reset_str = "\x1b[0m";
const char *bold_str = "\x1b[1m";
const char *italic_str = "\x1b[3m";
const char *underline_str = "\x1b[4m";

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

  /* Read File to get size */
  fp = fopen(fileName, "rb");
  if (fp == NULL) {
    return "";
  }
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp) + 1;
  fclose(fp);

  /* Read File for content */
  fp = fopen(fileName, "r");
  content = (char *)memset(malloc(size), '\0', size);
  fread(content, 1, size - 1, fp);
  fclose(fp);

  return content;
}

// Create map to store captures
// Use traversal order to keep track of node
typedef struct {
  TSNode n;
  int    i;               // index of node
  int    s;               // count of styles
  char   styles[16][128]; // for now some fixed number
} Node;

int count_nodes(TSNode n, int count) {
  int n_children = ts_node_child_count(n);
  count++;
  for (int i = 0; i < n_children; i++) {
    count = count_nodes(ts_node_child(n, i), count);
  }
  return count;
}

int index_nodes(TSNode n, Node *nodes, int count) {
  int n_children = ts_node_child_count(n);
  Node s = {0};
  s.n = n;
  s.i = count;
  nodes[count] = s;
  count++;
  for (int i = 0; i < n_children; i++) {
    count = index_nodes(ts_node_child(n, i), nodes, count);
  }
  return count;
}

int get_node_idx(TSNode n, Node *nodes, int n_nodes) {
  for (int i = 0; i < n_nodes; i++) {
    if (n.id == nodes[i].n.id) {
      return i;
    }
  }
  return -1;
}

// Types and corresponding themes
#define MAX_STYLES 128
typedef struct {
  char names[MAX_STYLES][64]; // Names of styles
  int  background[MAX_STYLES];
  int  foreground[MAX_STYLES];
  int  bold[MAX_STYLES];
  int  italic[MAX_STYLES];
  int  underline[MAX_STYLES];
  int  count;
} Theme;

TSNode get_theme_node(TSNode n, char *config_code, TSNode found) {
  uint32_t sb = ts_node_start_byte(n);
  uint32_t eb = ts_node_end_byte(n);
  char *text = get_substring(config_code, sb, eb);
  if (!strcmp(text, "theme")) {
    return ts_node_named_child(ts_node_parent(ts_node_parent(n)), 1);
  }
  free(text);
  int n_children = ts_node_child_count(n);
  for (int i = 0; i < n_children; i++) {
    found = get_theme_node(ts_node_child(n, i), config_code, found);
  }
  return found;
}

Theme get_theme(TSNode n, char *config_code) {
  Theme t = {0};
  t.count = ts_node_named_child_count(n);
  for (int i = 0; i < t.count; i++) {
    TSNode k = ts_node_named_child(ts_node_named_child(n, i), 0);
    TSNode v = ts_node_named_child(ts_node_named_child(n, i), 1);
    uint32_t sb, eb;

    sb = ts_node_start_byte(k);
    eb = ts_node_end_byte(k);
    // +-1 for quotes ("...")
    char *key = get_substring(config_code, sb + 1, eb - 1);
    sprintf(t.names[i], "%s", key);
    free(key);

    // If it is a number, set the foreground
    if (!strcmp(ts_node_type(v), "number")) {
      sb = ts_node_start_byte(v);
      eb = ts_node_end_byte(v);
      char *val = get_substring(config_code, sb, eb);
      t.foreground[i] = atoi(val);
      free(val);
    } else if (!strcmp(ts_node_type(v), "object")) {
      // Parse object to get individual properties
      for (int j = 0; j < ts_node_named_child_count(v); j++) {
        TSNode vv  = ts_node_named_child(v, j);
        TSNode vvk = ts_node_named_child(vv, 0);
        TSNode vvv = ts_node_named_child(vv, 1);
        sb = ts_node_start_byte(vvk);
        eb = ts_node_end_byte(vvk);
        // +-1 for quotes ("...")
        char *vkey = get_substring(config_code, sb + 1, eb - 1);
        sb = ts_node_start_byte(vvv);
        eb = ts_node_end_byte(vvv);
        char *vval = get_substring(config_code, sb, eb);
        if (!strcmp(vkey, "color")) {
          t.foreground[i] = atoi(vval);
        } else if (!strcmp(vkey, "background")) {
          t.background[i] = atoi(vval);
        } else if (!strcmp(vkey, "bold")) {
          t.bold[i] = 1;
        } else if (!strcmp(vkey, "italic")) {
          t.italic[i] = 1;
        } else if (!strcmp(vkey, "underline")) {
          t.underline[i] = 1;
        }
        free(vkey);
        free(vval);
      }
    }
  }
  return t;
}

void get_highlight(Theme *t, char *name) {
  int idx = -1;
  for (int i = 0; i < t->count; i++) {
    if (!strcmp(t->names[i], name)) {
      idx = i;
      break;
    }
  }

  // Print ansi string for given style
  if (idx >= 0) {
    if (t->bold[idx] > 0) {
      printf("%s", bold_str);
    }
    if (t->italic[idx] > 0) {
      printf("%s", italic_str);
    }
    if (t->underline[idx] > 0) {
      printf("%s", underline_str);
    }
    if (t->foreground[idx] > 0) {
      printf("\x1b[38;5;%dm", t->foreground[idx]);
    }
    if (t->background[idx] > 0) {
      printf("\x1b[48;5;%dm", t->background[idx]);
    }
  }
}

int print_tree(TSNode n, Node *nodes, int count, int n_nodes, char *content_file, Theme *t) {
  // Get text
  uint32_t sb = ts_node_start_byte(n);
  uint32_t eb = ts_node_end_byte(n);
  char *text = get_substring(content_file, sb, eb);

  // Check if next node has same start byte
  if (count < n_nodes) {
    TSNode next = nodes[count + 1].n;
    uint32_t nsb = ts_node_start_byte(next);
    if (nsb != sb) {
      // Get style
      if (nodes[count].s > 0) {
        get_highlight(t, nodes[count].styles[0]);
        printf("%s%s", text, reset_str);
      } else {
        printf("%s", text);
      }
      // Account for whitespace
      if (nsb > eb) {
        char *space = get_substring(content_file, eb, nsb);
        printf("%s", space);
        free(space);
      }
    }
  }
  free(text);

  count++;
  int n_children = ts_node_child_count(n);
  for (int i = 0; i < n_children; i++) {
    count =
        print_tree(ts_node_child(n, i), nodes, count, n_nodes, content_file, t);
  }

  return count;
}

const TSLanguage *tree_sitter_json();

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("USAGE: parse FILE_PATH HIGHLIGHTS_PATH CONFIG_PATH\n");
    return EXIT_FAILURE;
  }

  // Parse source_code
  char *file_path    = argv[1];
  char *content_file = get_content(file_path);

  // Load highlights from nvim-treesitter
  char *highlights_path    = argv[2];
  char *content_highlights = get_content(highlights_path);

  // Load config for styles from tree-sitter config file
  char *config_path    = argv[3];
  char *content_config = get_content(config_path);

  // Parse config file with json
  TSParser *config_parser = ts_parser_new();
  ts_parser_set_language(config_parser, tree_sitter_json());
  TSTree *config_tree = ts_parser_parse_string(config_parser, NULL, content_config, strlen(content_config));
  TSNode root_config  = ts_tree_root_node(config_tree);

  // Get "theme": {...} and parse
  TSNode root_theme   = {0};
  root_theme = get_theme_node(root_config, content_config, root_theme);
  if (root_theme.id == 0) {
    printf("Theme not found\n"); // TODO: make empty theme instead of crashing
    return EXIT_FAILURE;
  };
  Theme theme = get_theme(root_theme, content_config);

  // Parse file with go
  TSLanguage *tree_sitter_go();
  TSParser   *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_go());
  TSTree *tree = ts_parser_parse_string(parser, NULL, content_file, strlen(content_file));
  TSNode root  = ts_tree_root_node(tree);

  // Parse query for go
  TSQuery      *q;
  uint32_t     eo;
  TSQueryError et;
  q = ts_query_new(tree_sitter_go(), content_highlights, strlen(content_highlights), &eo, &et);
  if (et != TSQueryErrorNone) {
    printf("Error in `ts_query_new`: %lu\n", (uint64_t)et);
    return EXIT_FAILURE;
  }

  // Total number of query patterns, unique capture names and strings
  int n_patterns = ts_query_pattern_count(q);
  int n_captures = ts_query_capture_count(q);
  int n_strings  = ts_query_string_count (q);

  // For some reason, cannot use `node.id`, so keeping track manually
  int n_nodes = count_nodes(root, 0);

  // One style for each capture?
  Node *nodes = malloc(sizeof(Node) * n_nodes);
  index_nodes(root, nodes, 0);

  // Get captures from highlights queries
  TSQueryMatch  m;
  TSQueryCursor *c = ts_query_cursor_new();
  ts_query_cursor_exec(c, q, root);
  while (ts_query_cursor_next_match(c, &m)) {
    for (int ci = 0; ci < m.capture_count; ci++) {
      TSQueryCapture cp = m.captures[ci];
      TSNode  n     = cp.node;
      TSPoint start = ts_node_start_point(n);
      TSPoint end   = ts_node_end_point(n);

      // Get style name
      uint32_t length;
      const char *capture_name = ts_query_capture_name_for_id(q, cp.index, &length);

      // Check if node is in styles, and append
      int  idx = get_node_idx(n, nodes, n_nodes);
      Node *s  = &nodes[idx];
      sprintf(s->styles[s->s], "%s", capture_name);
      s->s++;
    }
  }

  // Finally, walk tree and print styles
  print_tree(root, nodes, 0, n_nodes, content_file, &theme);

  return EXIT_SUCCESS;
}

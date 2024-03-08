---
title: "Tools"
draft: true
date: 2024-01-01T08:00:00-08:00
---

Tree-sitter comes with 3 commonly used CLI tools: `parse`, `query` and `highlight`.

## Common

These parts are common to all 3 programs.

### data

For now, we only implement the programs for the language `go`. To support all
languages, I assume we would have to load our definitions from a `.so` file
rather than specify 100s of them at compile time.

Our test file is:

```go
package main

import "fmt"

func Add(a, b int) int { return a + b }

type Sum struct {
 a, b, c int
}

func (s Sum) String() string { return fmt.Sprintf("%d + %d = %d", s.a, s.b, s.c) }

type SumInterface interface {
 Add(a, b int) int
}
```

### imports

The only 'real' import we need is `<tree_sitter/api.h>`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>
```

To include it in compilation, here is an example Makefile (I use `bear` to make
the `compile_commands.json` file, not sure if there is an easier way.).

```bash
# Path to downloaded tree-sitter repos
TREE_SITTER_DIR = "/home/x/fd/code/tree-sitters"

parse: parse.c
 bear -- gcc                                          \
   -I tree-sitter/lib/include                       \
   parse.c                                          \
   $(TREE_SITTER_DIR)/tree-sitter/libtree-sitter.a  \
   $(TREE_SITTER_DIR)/tree-sitter-go/src/parser.c   \
   -g -o parse
```

### get_content

All three tools read an entire file to memory and then parse source file and/or
the queries file with tree-sitter. Accordingly:

```c
char *get_content(const char *fileName) {
  FILE *fp;
  long size = 0;
  char *content;

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
```

## Parse

The inbuilt CLI tool `parse` from tree-sitter has the following features:

```bash
> tree-sitter parse --help
tree-sitter-parse
Parse files

USAGE:
    tree-sitter parse [FLAGS] [OPTIONS] [--] [paths]...

FLAGS:
    -d, --debug          Show parsing debug log
    -0, --debug-build    Compile a parser in debug mode
    -D, --debug-graph    Produce the log.html file with debug graphs
        --dot
    -x, --xml
    -s, --stat           Show parsing statistic
    -t, --time           Measure execution time
    -q, --quiet          Suppress main output
    -h, --help           Prints help information
    -V, --version        Prints version information

OPTIONS:
        --paths <paths-file>    The path to a file with paths to source file(s)
        --scope <scope>         Select a language by the scope instead of a file extension
        --timeout <timeout>     Interrupt the parsing process by timeout (µs)
    -e, --edit <edits>...       Apply edits in the format: "row,col del_count insert_text"

ARGS:
    <paths>...    The source file(s) to use
```

### Debug options

To prevent being overwhelmed with information, we use the following `go` code,
saved to `cli/package_main.go` (one of the rare cases the filename is larger
than the actual file):

```go
package main
```

Running `tree-sitter parse cli/package_main.go` outputs:

```s-exp
(source_file [0, 0] - [15, 0]
  (package_clause [0, 0] - [0, 12]
    (package_identifier [0, 8] - [0, 12]))
```

Using the debug option `tree-sitter parse -d cli/package_go.go`, we get:

```bash
new_parse
process version:0, version_count:1, state:1, row:0, col:0
lex_internal state:59, row:0, column:0
  consume character:'p'
  consume character:'a'
  consume character:'c'
  consume character:'k'
  consume character:'a'
  consume character:'g'
  consume character:'e'
  consume character:'p'
  consume character:'a'
  consume character:'c'
  consume character:'k'
  consume character:'a'
  consume character:'g'
  consume character:'e'
lexed_lookahead sym:package, size:7
shift state:1363
process version:0, version_count:1, state:1363, row:0, col:7
lex_internal state:0, row:0, column:7
  skip character:' '
  consume character:'m'
  consume character:'a'
  consume character:'i'
  consume character:'n'
  consume character:'m'
  consume character:'a'
lexed_lookahead sym:identifier, size:5
shift state:1071
process version:0, version_count:1, state:1071, row:0, col:12
lex_internal state:56, row:0, column:12
  consume character:10
lexed_lookahead sym:\n, size:1
reduce sym:package_clause, child_count:2
shift state:241
process version:0, version_count:1, state:241, row:1, col:0
lex_internal state:59, row:1, column:0
lexed_lookahead sym:end, size:0
reduce sym:source_file_repeat1, child_count:2
reduce sym:source_file, child_count:1
accept
done
(source_file [0, 0] - [1, 0]
  (package_clause [0, 0] - [0, 12]
    (package_identifier [0, 8] - [0, 12])))
```

I have not had a chance to understand the output properly yet.

The run with graph looks even better: [log.html](/projects/tree-sitter/02-tools/log.html)

### Custom implementation

The following is our typical boilerplate to use tree-sitter:

```c
int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("USAGE: parse FILE_PATH\n");
    return EXIT_FAILURE;
  }
  char *file_path = argv[1];
  char *content   = get_content(file_path);

  // Keep language constant as `go` for now
  TSLanguage *tree_sitter_go();
  TSParser   *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_go());

  // Recursively print the tree
  TSTree *tree = ts_parser_parse_string(parser, NULL, content, strlen(content));
  TSNode root  = ts_tree_root_node(tree);
  print_tree(root, content, 0, NULL);
  printf("\n");

  return EXIT_SUCCESS;
}
```

For parsing, all we need is to walk the tree and print the nodes and their metadata in order:

```c
char *get_indent_str(int indent) {
  char *str = malloc(sizeof(char) * (indent + 1));
  for (int i = 0; i < indent; i++) {
    str[i] = ' ';
  }
  str[indent] = '\0';
  return str;
}

void print_tree(TSNode n, char *content, int indent, const char *field) {
  // Only print if named
  if (ts_node_is_named(n)) {
    TSPoint sp = ts_node_start_point(n);
    TSPoint ep = ts_node_end_point(n);
    char *indent_str = get_indent_str(indent);
    if (!field) {
      printf("%s (%s [%d, %d] - [%d, %d]", indent_str, ts_node_type(n), sp.row,
             sp.column, ep.row, ep.column);
    } else {
      printf("%s %s : (%s [%d, %d] - [%d, %d]", indent_str, field,
             ts_node_type(n), sp.row, sp.column, ep.row, ep.column);
    }
    free(indent_str);
  }

  // Iterate through all or we miss fields
  int n_children = ts_node_child_count(n);
  for (int i = 0; i < n_children; i++) {
    TSNode child = ts_node_child(n, i);
    if (ts_node_is_named(child)) {
      printf("\n");
    }
    const char *field_name = ts_node_field_name_for_child(n, i);
    print_tree(child, content, indent + 2, field_name);
  }

  // Print only if named
  if (ts_node_is_named(n)) {
    printf(")");
  }
}
```

Running `./cli/parse cli/package_main.go`, we reproduce the output:

```s-exp
(source_file [0, 0] - [1, 0]
  (package_clause [0, 0] - [0, 12]
    (package_identifier [0, 8] - [0, 12])))
```

## Query

From the website:

A query consists of one or more patterns, where each pattern is an S-expression
that matches a certain set of nodes in a syntax tree. The expression to match a
given node consists of a pair of parentheses containing two things: the node’s
type, and optionally, a series of other S-expressions that match the node’s
children.

### Native implementation

The inbuilt CLI tool `parse` from tree-sitter has the following features:

```bash
> tree-sitter query --help
tree-sitter-query
Search files using a syntax tree query

USAGE:
    tree-sitter query [FLAGS] [OPTIONS] <query-path> [paths]...

FLAGS:
    -t, --time        Measure execution time
    -q, --quiet       Suppress main output
    -c, --captures
        --test
    -h, --help        Prints help information
    -V, --version     Prints version information

OPTIONS:
        --paths <paths-file>         The path to a file with paths to source file(s)
        --byte-range <byte-range>    The range of byte offsets in which the query will be executed
        --row-range <row-range>      The range of rows in which the query will be executed
        --scope <scope>              Select a language by the scope instead of a file extension

ARGS:
    <query-path>    Path to a file with queries
    <paths>...      The source file(s) to use
```

Queries are expressed in the language defined in [Query
Syntax](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).

We observe that the queries have similar syntax to the output of `parse`.

So lets just use the output of parse as our query, after removing all the extras.

```bash
(source_file
 (package_clause
   (package_identifier) @name))
```

This results in the following output:

```bash
> tree-sitter query cli/package_query.scm cli/package_main.go
cli/package_main.go
  pattern: 0
    capture: 0 - name, start: (0, 8), end: (0, 12), text: `main`
```

This time, we don't have any debug outputs.

### Custom implementation

We need this utility to print the text as the output of `query` does in cases
where the text fits on one line.

```c
char *get_substring(char *line, int start, int end) {
  int len = end - start;
  char *s = malloc(sizeof(char) * (len + 1));
  for (int i = 0; i < len; i++) {
    s[i] = line[start + i];
  }
  s[len] = '\0';
  return s;
}
```

The implementation this time doesn't even need recursion, we just iterate
through the captures and print out the relevant information.

```c
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
```

As expected, we reproduce the output of the CLI tool:

```bash
> ./cli/query cli/package_query.scm cli/package_main.go
cli/package_main.go
  pattern: 0
    capture: 0 - name, start: (0, 8), end: (0, 12), text: `main`
```

## Highlight

### Native implementation

Tree-sitter `highlight`:

```bash
tree-sitter-highlight
Highlight a file

USAGE:
    tree-sitter highlight [FLAGS] [OPTIONS] [paths]...

FLAGS:
    -H, --html       Generate highlighting as an HTML document
    -t, --time       Measure execution time
    -q, --quiet      Suppress main output
    -h, --help       Prints help information
    -V, --version    Prints version information

OPTIONS:
        --scope <scope>         Select a language by the scope instead of a file extension
        --paths <paths-file>    The path to a file with paths to source file(s)

ARGS:
    <paths>...    The source file(s) to use
```

We cannot show the terminal coloring here as that uses ANSI escape codes, but,
fortunately, it produces html output, so we can just paste that here.

```html
> tree-sitter highlight --html cli/package_main.go
<!doctype html>
<head>
  <title>Tree-sitter Highlighting</title>
  <style>
    body {
      font-family: monospace;
    }
    .line-number {
      user-select: none;
      text-align: right;
      color: rgba(27, 31, 35, 0.3);
      padding: 0 10px;
    }
    .line {
      white-space: pre;
    }
  </style>
</head>
<body>
  <table>
    <tr>
      <td class="line-number">1</td>
      <td class="line"><span style="color: #5f00d7">package</span> main</td>
    </tr>
  </table>
</body>
```

However, it produces a whole page, so let us also clean that up by removing
the head, title and body tags and preserving style and table.

```html
> tree-sitter highlight --html cli/package_main.go
<style>
  ts-highlight {
    font-family: monospace;
  }
  .line-number {
    user-select: none;
    text-align: right;
    color: rgba(27, 31, 35, 0.3);
    padding: 0 10px;
  }
  .line {
    white-space: pre;
  }
</style>

<div class="ts-highlight">
  <table>
    <tr>
      <td class="line-number">1</td>
      <td class="line"><span style="color: #5f00d7">package</span> main</td>
    </tr>
  </table>
</div>
```

Finally, we can show it here as:

<style>
  ts-highlight {
    font-family: monospace
  }
  .line-number {
    user-select: none;
    text-align: right;
    color: rgba(27,31,35,.3);
    padding: 0 10px;
  }
  .line {
    white-space: pre;
  }
</style>

<div class="ts-highlight"> <table>
<tr><td class=line-number>1</td><td class=line><span style='color: #5f00d7'>package</span> main
</td></tr>
</table> </div>

The styles are stored in a JSON config at `~/.config/tree-sitter/config.json`.

```json
{
  "parser-directories": [
    "/home/x/fd/code/tree-sitters"
  ],
  "theme": {
    "comment": {
      "italic": true,
      "color": 245
    },
    "constant.builtin": {
      "bold": true,
      "color": 94
    },
    "string.special": 30,
    "tag": 18,
    ...
  }
}
```

As we can see some styles are predefined.

### Custom implementation

We need to first parse the JSON config, for which we use `tree_sitter_json()` itself.

```c
// Types and corresponding themes
#define MAX_STYLES 128
typedef struct {
  char names      [MAX_STYLES][64];
  int  background [MAX_STYLES];
  int  foreground [MAX_STYLES];
  int  bold       [MAX_STYLES];
  int  italic     [MAX_STYLES];
  int  underline  [MAX_STYLES];
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
```

Next, we traverse the tree and keep track of the captures corresponding to each
node. For some reason, I could not use `node.id` and so I decided to keep track
of the nodes manually, where each node can have a maximum of 16 styles.

```c
// Create map to store captures
// Use traversal order to keep track of node
typedef struct {
  TSNode n;
  int    i;               // index of node
  int    s;               // count of styles
  char   styles[16][128]; // for now some fixed number
} Node;

int get_node_idx(TSNode n, Node *nodes, int n_nodes) {
  for (int i = 0; i < n_nodes; i++) {
    if (n.id == nodes[i].n.id) {
      return i;
    }
  }
  return -1;
}

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
```

In order to recreate this for the terminal, we need to use ANSI escape codes.
Now we walk the tree, get the corresponding highlight and print the style,
making sure to print whitespace and resets correctly.

```c
const char *reset_str = "\x1b[0m";
const char *bold_str = "\x1b[1m";
const char *italic_str = "\x1b[3m";
const char *underline_str = "\x1b[4m";

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
```

So putting it all together, we get:

```c
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
```

<hr/>

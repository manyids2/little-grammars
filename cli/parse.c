#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("USAGE: parse FILE_PATH\n");
    return EXIT_FAILURE;
  }
  const char *file_path = argv[1];
  char *content = get_content(file_path);

  // Keep language constant as `go` for now
  TSLanguage *tree_sitter_go();
  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_go());

  // Recursively print the tree
  TSTree *tree = ts_parser_parse_string(parser, NULL, content, strlen(content));
  TSNode root = ts_tree_root_node(tree);
  print_tree(root, content, 0, NULL);
  printf("\n");

  return EXIT_SUCCESS;
}

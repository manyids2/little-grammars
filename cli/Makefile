TREE_SITTER_DIR = "/home/x/fd/code/tree-sitters"

parse: parse.c
	bear -- gcc                                        \
	 	-I tree-sitter/lib/include                       \
	 	parse.c                                          \
	 	$(TREE_SITTER_DIR)/tree-sitter/libtree-sitter.a  \
	 	$(TREE_SITTER_DIR)/tree-sitter-go/src/parser.c   \
	 	-g -o parse

query: query.c
	bear -- gcc                                        \
	 	-I tree-sitter/lib/include                       \
	 	query.c                                          \
	 	$(TREE_SITTER_DIR)/tree-sitter/libtree-sitter.a  \
	 	$(TREE_SITTER_DIR)/tree-sitter-go/src/parser.c   \
	 	-g -o query

highlight: highlight.c
	bear -- gcc                                        \
	 	-I tree-sitter/lib/include                       \
	 	highlight.c                                      \
	 	$(TREE_SITTER_DIR)/tree-sitter/libtree-sitter.a  \
	 	$(TREE_SITTER_DIR)/tree-sitter-go/src/parser.c   \
	 	$(TREE_SITTER_DIR)/tree-sitter-json/src/parser.c \
	 	-g -o highlight

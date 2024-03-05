.PHONY: docs

docs:
	rsync -avr --delete \
		/home/x/fd/code/llm/little-grammars/docs/  \
		/home/x/blogs/manyids2/content/projects/tree-sitter/

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

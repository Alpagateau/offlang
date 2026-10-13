# Syntax idea

```c
pure increment(int a)
{
  return a + 1;
}

int main()
{
  int[-3, 8] a = 9; // compiled, but warns (clamps to 8)
  
  if a > 3 {
    a = increment(a);
  }
  
  if(a < 10) { // Always true 
    a = increment(a);
  } else { // Only branch kept here
    a = 0;
  }
  
  return 0;
}
```


# What's needed to implement this

- expression parser (x)
- statement list / block ( )

A statement could be either : 
- declaration        (type name | type name (= expr)?) (done (but no types yet))
- an assignment      (name = expr)
- an if statement    (if expr statement (else statement)?) (done)
- a while statement  (while expr statement) (done)
- a defer statement  (defer statement)
- a block statement  ({ list of statements })


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
- statement list / block (x)

A statement could be either : 
- declaration        (type name | type name (= expr)?)     (done (but no types yet))
- an assignment      (name = expr)                         (done)
- an if statement    (if expr statement (else statement)?) (done)
- a while statement  (while expr statement)                (done)
- a defer statement  (defer statement)                     (done)
- a block statement  ({ list of statements })              (done)

A type could be either:
- default type (int currently) (done)
- bound type (int<number, number>) (done)

# First iterations :
- function definitions 
- function calls
- pointers 
- char types

## Maybe later
Implement the type system so that a each type is a singleton. Only one pointer for the type int exists, etc.... (better than allocating new memory each time)

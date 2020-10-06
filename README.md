# DM14
the Distributed M14 programming language

### Introduction

This is a novel distributed programming language that I wrote during my bachelor, while the syntax is always the same, the compiler has went through many refactoring and still is.

### Examples

The examples directory has a set of "working" examples 

#### Factorial RPC like

```cpp

with io use io

factorial (n int -> nFactorial float);

main ( -> x int)
{
  input int;
  result backprop float;
  spill("please enter input:"); get(input);
  spill("factorial input :"); nspill(result);

  distribute;

  result = factorial(input);
  return;
}



factorial (n int  -> nFactorial float)
{
  if [n < 0]  
  {
    nspill("Number is a negative, converting to positive.");
    n = n * -1 ;
  }

  nFactorial = n;

  for [k int =0; k < n- 1; k++;]  
  {
    nFactorial *= ((k+1));
  }
  return;
}
```
# References

##### Master's Thesis : 
https://www.researchgate.net/publication/329183467_Distributed_coastal_erosion_management_using_image_processing_and_Node_Oriented_Programming
#####  Master's Presentation : 
https://www.researchgate.net/publication/329183467_Distributed_coastal_erosion_management_using_image_processing_and_Node_Oriented_Programming

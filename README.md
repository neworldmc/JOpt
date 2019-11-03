# JOpt
Java Bytecode Optimize: Parse, Analyze, Patch

## Principle
JOpt is a radical underlying optimizer. It performs optimization on the 3-address code layer, and it doesn't guarantee the internal structure of optimized code.

During an optimization work, JOpt **parse**s the .class files and generate some graphs first. Then JOpt **analyze**s these graphs and generates optimization patches. Finally, JOpt **patch**es previous .class files using these generated patches.

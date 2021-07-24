# Persistent_Checkpointing
The following contains the code for copying tensors to persistent memory. The main code resides in PyPmem/Obj/pmem_pool.c. 
Any additional programs were created during development but are provided for reference.

The file PyPmem/file.py is the file.py used in the Amazon Sagemaker Debugger. This file was modified to integrate Amazon Sagemaker Debugger
with peristent memory. Be sure to update this file with the corresponding changes when testing with the Amazon Sagemaker Debugger. More
information can be found under the Persistent Memory Documentation pdf.

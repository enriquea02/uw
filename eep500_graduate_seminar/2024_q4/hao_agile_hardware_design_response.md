As a PMP student, I work full-time as a firmware engineer at Blue Origin implementing hardware architecture designs on FPGAs. The Agile Hardware Development: Architectures and Tools presentation by Dr. Cong "Callie" Hao hit on multiple challenges like bad tools, inprecise resource utilization by tools, mismatches between behavior in hardware and simulation, and. Before elaborating, the quick overview of her research was interesting and does address industry challenges, but from my perspective in industry there are "misses" in her research from my experience in industry. Primarly, her research seems to abstract the underlying hardware that is worked with, which has been an industry goal for decades now with products such as Vitis, which translates C/C++ code into RTL. Although I see the benefits of this, since it allows for software-centric (Agile) hardware development, the main drawback is that the hardware is abstracted away. From my experience, there are company-specific, even product-specific features, that make two FPGAs have different underlying fabric architecture. Within my own team, I've worked with FPGAs from Lattice Semiconductos and Microchip, and the two FPGAs have different reset structures and fanouts in the fabric, which force us designers to approach our RTL designs differently to optimize for the underlying ASIC fabric in the FPGA. 

### Overview of Disagreement with her Research
- I ASSUME her research abstracts the hw fabric away <br>

### Summarize talk
- Arch
- Tool development

### Application of Her Research in Industry
- Architecture solutions help with processing data where latency is key
- Tools seem very good for quick prototype and development
- Her RealProbe application is very good for debugging. This is an industry struggle, because many things ARE black boxes and teams have to tangentially verify applications
    - e.g. how do you verify that your writes to memory are indeed being written to the address YOU think you're writing to

### Future Extensions of the Work
- Improved quick simulation prior to RTL creation
- Improved architecture for niche applications where data processing latency must fit within run-time application processing and packaging data for slower processing or offline processing
- Express where I think her research is applicable in my life
Before elaborating on the benefits and drawbacks I noted during presentation, I will summarize the work she presented, use case applications that were presented as well as my own, and next steps that were highlighted. 

### Citation
# SecTrans for MacroHard

# how do i compile ?
```
make
```

# how is this supposed to work and what makes it secure ?
## AUTHENTICATION !!1!1
for you to be able to do anything on the client app (i.e upload a file, download a file, or even just see that the files in our prestige server are) you need to be authenticated, meaning any time you run secTrans you need to provide a username and password.
## Hashing
when you upload a file the client app will also upload a hash of that file. meaning that whenever it is downloaded it can be compared next to it's hash to see if it's the file we want or not.
## Encryption!!!
we will use some sort of asymetric public/private encryption that is end to end to ensure the sec in trans (unlike whatsapp)

### Pay attention! Development will be done on separate branches and merged into develop. then when all works as we want it to work, we can merge it into main.

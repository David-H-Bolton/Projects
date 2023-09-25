import itertools
import os
import requests
from bs4 import BeautifulSoup as bs
import time

s = requests.Session()
errors = []

# Fetch url into filename and wait 20 seconds
def fetch(url,filename):
    print(f"Fetching {url} into {filename}")
    try:
        r = s.get(url)
        status = r.status_code
        if r.ok:
            textfile = open(filename,"w")
            textfile.write(r.text)    
        else:
            errors.append(f"Failed to fetch {url} - status {status}")

    except Exception as err:
        errors.append(F"Failed to read {url} because {err}")                

first=67
last=306
folder = 'html';
numbers = list(itertools.chain(range(first,last)))

rawurl='https://linuxformat.com/archives?issue='
headers=["Features","Tutorials","Coding Academy","Reviews"]

filename="lf.txt"
csvtextfile = open(filename,"a")
 
for n in numbers:
    url = f"{rawurl}{n}"
    filename = f"{n}.html"
    filename = os.path.join(folder,filename)
    if not os.path.isfile(filename): 
        fetch(url,filename)
        time.sleep(20)
if len(errors) > 0:
    print(f"Errors found {len(errors)}")
    print(errors)
    exit(1)
#Parsing code
for n in numbers:
    filename = f"{n}.html"
    filename = os.path.join(folder,filename)
    if os.path.join(folder,filename):
        textfile = open(filename,"r")
        text=textfile.read()    
        body=bs(text,"lxml")
        # find all h2 first
        issue=0
        allh2s = body.findAll("h2")
        for h2 in allh2s:
            if h2:            
                h2s= h2.string
                h2split = h2s.split()
                if len(h2split) >1: # Issue,n,
                    issue = h2split[1]
                if h2split[0] in headers:
                    if len(h2split) ==1: 
                        div=h2.find_next() # div following h2                 
                        text = div.h3.string
                        text2 = div.find('p')
                        text = f"{text},{issue},{text2.contents[0]}"
                        csvtextfile.write(text+"\n")
csvtextfile.close()
print("Finished")
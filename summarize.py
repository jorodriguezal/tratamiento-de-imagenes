#!/bin/python

import pandas as pd
d=pd.read_csv('results_graphs.txt', sep=';')
n=int(len(d)/15)
trim=5
threads = [1, 2, 4, 8, 16]
trimmed = []

for im in ['720.jpg', '1080.jpg', '4k.jpg']:
    trimmed += [d[(d.threads == i) & (d.image == im)].sort_values('seconds')[trim:n-trim] for i in threads]
temp = trimmed[0]

for i in range(1, len(trimmed)):
    temp = temp.append(trimmed[i])

print(temp.groupby([temp.image, temp.threads]).describe().to_csv('summary.csv'))

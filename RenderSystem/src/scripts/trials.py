
# print ("Hello world")
# name = input('whats your name: ')
# age = input('whats your age ')
# print(name.upper(),' age is: ', age)

# ageflt = float(age)

# print('age in flt is: ', str(ageflt))

from os import name


names = ['hello', 'my', 'name', 'is', "sid"]
for mystr in names:
  print(mystr)  
  
print(names[-1])
print(names[1:3])

names.insert(0, 'james')
print(names)
names.remove('james')
print(names)
print('num elems: ', len(names))


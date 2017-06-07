import re, string, timeit
 
s = "string. With. Punctuation"
regex = re.compile('[%s]' % re.escape(string.punctuation))
exclude = set(string.punctuation)
table = string.maketrans("","")
 
def test_set(s):
    #exclude = set(string.punctuation)
    return ''.join(ch for ch in s if ch not in exclude)
 
def test_re(s):  # From Vinko's solution, with fix.
    #regex = re.compile('[%s]' % re.escape(string.punctuation))
    return regex.sub('', s)
 
def test_trans(s):
    #table = string.maketrans("","")
    return s.translate(table, string.punctuation)
 
def test_repl(s):  # From S.Lott's solution
    for c in string.punctuation:
        s=s.replace(c,"")
    return s
 
print "sets      :",timeit.Timer('f(s)', 'from __main__ import s,test_set as f').timeit(1000000)
print "regex     :",timeit.Timer('f(s)', 'from __main__ import s,test_re as f').timeit(1000000)
print "translate :",timeit.Timer('f(s)', 'from __main__ import s,test_trans as f').timeit(1000000)
print "replace   :",timeit.Timer('f(s)', 'from __main__ import s,test_repl as f').timeit(1000000)
 
## -- End pasted text --
#sets      : 3.36192652878
#regex     : 1.26144765326
#translate : 0.436933142708
#replace   : 6.08965365232
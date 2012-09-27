import cphvbnumpy as numpy
import cphvbbridge
import util
    
def main():

    b = util.Benchmark()
    n = b.size[0]
    i = b.size[1]

    raw = numpy.empty((n+4))
    raw.cphvb = b.cphvb

    tmp = numpy.empty((n))
    tmp.cphvb = b.cphvb

    center  = raw[2:-2]
    left1   = raw[1:-3]
    left2   = raw[ :-4]
    right1  = raw[3:-1]
    right2  = raw[4:  ]

    center += 1.0
    #tmp = center # Death!
    cphvbbridge.flush()
    print "start"
    b.start()

    for _ in xrange(i):
        tmp[:] = center
        #tmp = center
        tmp += left1
        tmp += left2
        tmp += right1
        tmp += right2
        tmp /= 9
        center[:] = tmp
        #center = tmp

    b.stop()
    b.pprint()

if __name__ == "__main__":
    main()

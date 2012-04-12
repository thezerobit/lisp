(def map (lambda (f l)
           (if l (cons (f (first l)) (map f (rest l))))))

(def range (lambda (a b)
             (if (< a b)
               (cons a (range (+ a 1) b)))))

(def a (lambda (x) (if (<= x 0) x (a (- x 1)))))
(letrec (b (lambda (x) (if (<= x 0) x (b (- x 1))))) (a 100))

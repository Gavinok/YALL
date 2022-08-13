(quote (Hello))
;; (sleep 3)
(quote (Welcome a quick video demo of YALL))
;; (sleep 3)
(quote (YALL is a simple scheme like language written in C++ intended to ))
(quote (make extending existing C++ libraries))
;; (sleep 10)
(quote (So what can YALL do))
;; (sleep 4)
(quote (You can lists useing the LIST function like so (in lower case)))
;; (sleep 2)
(quote (list 1 2 3))
(list 1 2 3)
;; (sleep 10)
(quote (you can access these elements using useing the CAR and CDR functions))
;; (sleep 2)
(quote (car (list 1 2 3)))
(quote (will result in))
;; (sleep 1)
(car (list 1 2 3))
;; (sleep 1)
(quote (and))
(quote (car (list 1 2 3)))
(quote (will result in))
;; (sleep 1)
(cdr (list 1 2 3))

(quote (1 . 2))
(quote (a . b))
(quote (a . (1 . b)))
;; TODO figure out what is oing wrong here
;; (quote ((a . hello) . (1 . b)))


(quote (a . (b . (c . (quote ())))))

(list (quote a) (quote b) (quote c))

(quote  (+ 1 2))
(+ 1 2)

(quote (cons 1 2))
(cons 1 2)

(quote (quote  (1 . 2)))


(quote (quote a))

(quote (car (cons 1 2)))
(car (cons 1 2))

(quote (cdr (cons 1 2)))

(cdr (cons 1 2))

(quote (define x 1))


(define x 1)


(quote (+ x 1))
(+ x 1)

(quote (define y 10))
(define y 10)
(quote (+ y 1))

(+ y 1)

(quote (let ((x 1)
             (y 9))
         (+ x y)))

(let ((x 1)
      (y 9))
  (+ x y))


(let ((x 1)
      (y 9))
  (let ((x 5))
    (+ x y)))

(quote (lambda (x) (+ 9 x)))
(lambda (x) (+ 9 x))


(quote ((lambda (x) (+ 9 x)) 2))
((lambda (x) (+ 9 x)) 2)

(define adder (lambda (x) (+ 9 x)))
(adder 2)

(quote (let ((adder (lambda (x) (+ 9 x))))
         (adder 2)))

(let ((adder (lambda (x) (+ 9 x))))
  (adder 2))

(quote (define x (let ((x  1))
                   (lambda (y)
                     (+ x y)))))

(define x (let ((x  1))
            (lambda (y)
              (+ x y))))

(quote (x 2))
(x 2)


(quote (if (= 2 1)
           (+ 1 2)
           (- 1 2)))
(if (= 2 1)
    (+ 1 2)
    (- 1 2))

(eq 1 1)
(eq (quote a) (quote a))
(eq (quote a) (quote b))
(eq (quote a) 1)
(eq #t #t)
(eq #t #f)


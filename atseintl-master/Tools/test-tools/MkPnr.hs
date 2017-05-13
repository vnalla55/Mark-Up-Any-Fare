module Main where

import IO

import Text.XML.HaXml.Types
import Text.XML.HaXml.Parse
import Text.XML.HaXml.Combinators
import Text.XML.HaXml.Html.Generate
import Text.XML.HaXml.Wrappers

main = do
    (inf,ouf) <- fix2Args
    input     <- if inf=="-" then getContents else readFile inf
    output    <- if ouf=="-" then return stdout else openFile ouf WriteMode
    parse     <- return (xmlParse inf)

    applyFilter input output parse (mkpnr `o` deep (tag "PricingRequest"))
    applyFilter input output parse (mkpnr `o` deep (tag "AltPricingRequest"))

    where
        applyFilter input output parse f = do
            (disp output . onContent f . parse . dropWhile (/='<')) input

mkpnr = 
    cat
        [ none
        , literal "I\n"
        , (avalf ("AAA"++) "A20") `o` (deep (tag ("AGI")))

        , catno `oo` numbered (deep (tag "IDS"))
        , literal "\n"

        , (avalf (takeWhile (/='+')) "S14") `o` (deep (tag ("PRO")))
        , literal "\n"
        ]

catno n =
    mkElem "FLI"
      [ 
        (attrval ("N03", AttValue[Left "K"]))
        ?>
        cat [ (literal "0"), (literal " ARUNK") ]
        :>
        cat [ literal "0" -- , (n!), literal " "
            , avalf (takeWhile (/=' ')) "B00"
            , avalf formatFlight "Q0B"
            , avalf (takeWhile (/=' ')) "B30"
            , avalf formatDate "D01"
            , aval "A01"
            , aval "A02"
            -- , (literal " GK") --
            , avalf (" "++) "A70"
            , avalf (dropWhile (=='0')) "Q0U"
            , literal " / "
            , aval "D31"
            , literal " "
            , aval "D32"
            ]
      ]

aval n    = iffind n (\a -> (a!)) (""!)

avalf f n = iffind n (\a -> ((f a)!)) (""!)

onContent :: CFilter -> Document -> [Content]
onContent filter (Document p s e m) = filter (CElem e)

disp o [] = return ()
disp o (c:cs) = dispContent o c >> disp o cs

dispContent o (CElem e)      = hPutStrLn o "" >> pElement o e
dispContent o (CString b cs) = hPutStr o cs
dispContent o c              = return ()

pElement o (Elem n as cs) = mapM_ (dispContent o) cs

formatFlight s = (concat (take (4 - (length s)) (repeat " "))) ++ s

data Mon = ZAN | JAN | FEB | MAR | APR | MAY | JUN
               | JUL | AUG | SEP | OCT | NOV | DEC
    deriving (Enum, Show)

formatDate ds = " " ++ day ++ (show month) ++ " "
    where day = drop 8 ds
          month = toEnum ((read . take 2 . drop 5) ds) :: Mon


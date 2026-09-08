## (skint date) — calendar dates

Date objects in the style of SRFI 19: a point in time expressed in the Gregorian
calendar, with a time zone offset. Conversions to and from the time objects of
[`(skint time)`](time.md) are provided, along with calendar queries and reading
and writing ISO 8601 timestamps.

```scheme
(import (skint date))
```

This library re-exports the whole of `(skint time)`, so importing it alone is
enough to work with both times and dates.

Together the two libraries are the foundation a full `(srfi 19)` will be built
on.

### Date objects

A date is immutable once created.

`(make-date nanosecond second minute hour day month year zone-offset)` → *date*

The components run from the smallest unit to the largest, followed by the zone
offset in seconds east of UTC.

```scheme
(make-date 0 0 0 0 1 1 1970 0)          ; the epoch
(make-date 0 0 30 10 15 6 2020 19800)   ; 2020-06-15 10:30 in +05:30
```

The ranges are those of SRFI 19: nanosecond 0 to 999999999, second 0 to 60 with
60 denoting a leap second, minute 0 to 59, hour 0 to 23, day 1 to 31 as the month
allows, month 1 to 12 with 1 meaning January, and any integer year. It is an
error to supply a component outside its range, or a date that does not exist in
the calendar.

`(date? obj)` → *boolean*

`(date-nanosecond date)` → *integer*
<br>`(date-second date)` → *integer*
<br>`(date-minute date)` → *integer*
<br>`(date-hour date)` → *integer*
<br>`(date-day date)` → *integer*
<br>`(date-month date)` → *integer*
<br>`(date-year date)` → *integer*
<br>`(date-zone-offset date)` → *integer*

### Calendar queries

`(date-year-day date)` → *integer*

The ordinal day within the year; 1 January is 1.

```scheme
(date-year-day (make-date 0 0 0 0 31 12 2000 0))   ; => 366, a leap year
(date-year-day (make-date 0 0 0 0 31 12 1999 0))   ; => 365
```

The Gregorian century rule is observed, so 1900 is not a leap year while 2000 is.

`(date-week-day date)` → *integer*

The day of the week, Sunday 0 through Saturday 6.

```scheme
(date-week-day (make-date 0 0 0 0 1 1 1970 0))   ; => 4, a Thursday
(date-week-day (make-date 0 0 0 0 1 1 2000 0))   ; => 6, a Saturday
```

`(date-week-number date day-of-week-starting-week)` → *integer*

The ordinal week of the year holding this date, where the second argument names
the day the week begins on, in the same numbering as `date-week-day`. A partial
week at the start of the year is not counted.

```scheme
(date-week-number (make-date 0 0 0 0 2 1 2000 0) 0)   ; => 2, weeks from Sunday
(date-week-number (make-date 0 0 0 0 2 1 2000 0) 1)   ; => 1, weeks from Monday
```

`(current-date [zone-offset])` → *date*

The current date, in the local time zone unless an offset is given.

### Converting between dates and times

`(date->time-utc date)` → *time*
<br>`(date->time-tai date)` → *time*
<br>`(date->time-monotonic date)` → *time*

`(time-utc->date time [zone-offset])` → *date*
<br>`(time-tai->date time [zone-offset])` → *date*
<br>`(time-monotonic->date time [zone-offset])` → *date*

The zone offset defaults to the local one. It is an error to hand one of the
`time-…->date` procedures a time whose type is not the one named.

```scheme
(time-second (date->time-utc (make-date 0 0 0 0 1 1 2000 0)))   ; => 946684800
```

Converting a date to a time and back returns the same date, given the same zone
offset.

Leap seconds are visible through the TAI conversions. The second inserted at the
end of 1998 is a date in its own right:

```scheme
(date->iso-8601 (time-tai->date (make-time time-tai 0 (+ 915148800.0 31)) 0))
; => "1998-12-31T23:59:60Z"
```

That second and the one that follows it share a single UTC instant, since UTC is
what the leap second is inserted into.

### ISO 8601 timestamps

Two procedures read and write timestamps in the profile of ISO 8601 used by RFC
3339. They are not part of SRFI 19, which specifies the different
`date->string` and `string->date` pair.

`(date->iso-8601 date)` → *string*

```scheme
(date->iso-8601 (make-date 0 0 0 0 1 1 1970 0))
; => "1970-01-01T00:00:00Z"

(date->iso-8601 (make-date 123456789 30 45 12 25 12 2023 0))
; => "2023-12-25T12:45:30.123456789Z"

(date->iso-8601 (make-date 0 0 30 10 15 6 2020 19800))
; => "2020-06-15T10:30:00+05:30"
```

The shape is always `YYYY-MM-DDThh:mm:ss`, followed by a fractional part when the
nanosecond is not zero, followed by the zone. All fields are zero padded, the
year to four digits. A zero offset is written `Z`; any other is written `+hh:mm`
or `-hh:mm`. When present, the fraction always carries nine digits.

Years outside four digits take an explicit sign: `-0001-01-01T00:00:00Z` for the
year before 1 CE, `+12345-01-01T00:00:00Z` beyond 9999.

It is an error if the argument is not a date.

`(iso-8601->date string)` → *date*

```scheme
(date-year (iso-8601->date "2023-12-25T12:45:30Z"))            ; => 2023
(date-zone-offset (iso-8601->date "2020-06-15T10:30:00+05:30")); => 19800
(date-nanosecond (iso-8601->date "2023-12-25T12:45:30.5Z"))    ; => 500000000
```

The parser accepts what the writer produces, and a little more:

- `T`, lowercase `t`, or a space may separate the date from the time.
- `Z` or lowercase `z` denotes UTC; an omitted zone is read as UTC.
- A fractional part may carry any number of digits and is scaled accordingly, so
  `.5` is 500000000 nanoseconds and `.123` is 123000000.
- A leap second, `:60`, is accepted.
- The year may carry a leading `+` or `-`, and may exceed four digits.

It is an error if the argument is not a string, or does not parse as a timestamp
of this form. A zone offset must be written with a colon, as `+05:30`; the
compact `+0530` is not accepted.

Reading a timestamp and writing it again reproduces the original string, for
every form the writer produces:

```scheme
(date->iso-8601 (iso-8601->date "2020-06-15T10:30:00+05:30"))
; => "2020-06-15T10:30:00+05:30"
```

### How numbers are represented

Date components are small enough to be exact integers in any build. The second
counts of the time objects these procedures produce and consume are not, and
follow the rule described under [`(skint time)`](time.md#how-numbers-are-represented):
whole flonums in a build without the numeric tower, exact integers with it.

The practical consequence for date code is confined to writing literal time
values, which should use flonums — `915148800.0` — so that they read in either
configuration.

### Relationship to SRFI 19

This library implements the date half of SRFI 19, together with the time half in
[`(skint time)`](time.md).

Provided beyond the SRFI: `date->iso-8601` and `iso-8601->date`.

Not provided here: `date->string` and `string->date` with their conversion
specifiers, and everything concerning Julian Day and Modified Julian Day numbers
— `date->julian-day`, `julian-day->date`, `current-julian-day` and the rest.

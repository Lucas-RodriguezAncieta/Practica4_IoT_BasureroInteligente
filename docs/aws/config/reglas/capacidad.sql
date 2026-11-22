
SELECT topic(2) AS thingName,
       fillPercent
FROM 'basurero/+/telemetria'
WHERE NOT isUndefined(fillPercent)

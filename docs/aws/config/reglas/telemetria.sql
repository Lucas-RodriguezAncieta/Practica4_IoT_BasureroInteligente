
SELECT *,
       topic(2) AS thingName,
       'TELEMETRIA' AS recordType,
       timestamp() AS receivedAt
FROM 'basurero/+/telemetria'

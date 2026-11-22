
SELECT *,
       topic(2) AS thingName,
       'EVENTO' AS recordType,
       timestamp() AS receivedAt
FROM 'basurero/+/eventos'

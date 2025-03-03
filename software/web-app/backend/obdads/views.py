from django.shortcuts import render
from rest_framework.response import Response
from rest_framework.decorators import api_view


@api_view(["GET"])
def liveData(request):
    # We would be getting live data from the OBD device here
    # Construct static data for now
    data = {
        "speed": 100,
        "rpm": 2000,
        "temperature": 50,
        "fuel_level": 50,
        "errors": [
            {
                "code": "P0301",
                "severity": "high",
                "description": "Cylinder 1 Misfire Detected",
            },
            {
                "code": "P0302",
                "severity": "low",
                "description": "Cylinder 2 Misfire Detected",
            },
        ],
    }
    return Response({"message": "Hello, this is live data", "data": data})
